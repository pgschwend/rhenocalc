#include "traceroute.h"

#include <QHostInfo>
#include <QElapsedTimer>
#include <QProcess>
#include <QRegularExpression>

#ifdef Q_OS_WIN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <icmpapi.h>
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")
#endif

#ifdef Q_OS_LINUX
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <poll.h>
#include <errno.h>
#include <cstring>
#endif

// ─────────────────────────────────────────────────────────────────────────────
// TracerouteWorker
// ─────────────────────────────────────────────────────────────────────────────

TracerouteWorker::TracerouteWorker(const QString& host, int maxHops, int timeout)
    : m_host(host), m_maxHops(maxHops), m_timeout(timeout) {
}

TracerouteWorker::~TracerouteWorker() {
#ifdef Q_OS_WIN
    if (m_icmpHandle) {
        IcmpCloseHandle(m_icmpHandle);
        m_icmpHandle = nullptr;
    }
#endif
}

bool TracerouteWorker::resolveHost() {
    // First try to parse as IP address directly
    if (m_targetAddress.setAddress(m_host)) {
        return true;
    }

    // Resolve hostname
    QHostInfo info = QHostInfo::fromName(m_host);
    if (info.error() != QHostInfo::NoError || info.addresses().isEmpty()) {
        emit error(QString("Could not resolve host: %1").arg(m_host));
        return false;
    }

    // Prefer IPv4
    for (const QHostAddress& addr : info.addresses()) {
        if (addr.protocol() == QAbstractSocket::IPv4Protocol) {
            m_targetAddress = addr;
            return true;
        }
    }

    m_targetAddress = info.addresses().first();
    return true;
}

void TracerouteWorker::start() {
    m_running = true;

    if (!resolveHost()) {
        emit finished(false, "Failed to resolve host");
        return;
    }

#ifdef Q_OS_WIN
    // Windows: Use ICMP API
    m_icmpHandle = IcmpCreateFile();
    if (m_icmpHandle == INVALID_HANDLE_VALUE) {
        emit error("Failed to create ICMP handle");
        emit finished(false, "ICMP initialization failed");
        return;
    }

    // Convert IP address to network byte order for IcmpSendEcho
    const quint32 hostIp = m_targetAddress.toIPv4Address();
    const IPAddr targetIp = htonl(hostIp);

    // Prepare request data
    const char sendData[] = "RhenoCalc Traceroute";
    const WORD sendSize = sizeof(sendData);

    // Allocate reply buffer
    const DWORD replySize = sizeof(ICMP_ECHO_REPLY) + sendSize + 8;
    auto* replyBuffer = new char[replySize];

    for (int ttl = 1; ttl <= m_maxHops && m_running; ++ttl) {
        TraceHop hop;
        hop.hop = ttl;

        IP_OPTION_INFORMATION options = {};
        options.Ttl = static_cast<UCHAR>(ttl);

        QElapsedTimer timer;
        timer.start();

        DWORD result = IcmpSendEcho(
            m_icmpHandle,
            targetIp,
            const_cast<LPVOID>(static_cast<const void*>(sendData)),
            sendSize,
            &options,
            replyBuffer,
            replySize,
            static_cast<DWORD>(m_timeout)
        );

        if (result > 0) {
            auto* reply = reinterpret_cast<PICMP_ECHO_REPLY>(replyBuffer);

            // Convert IP address
            in_addr addr;
            addr.S_un.S_addr = reply->Address;
            hop.address = QString::fromLatin1(inet_ntoa(addr));
            hop.rtt = static_cast<int>(reply->RoundTripTime);

            // Check if we reached the destination
            if (reply->Status == IP_SUCCESS) {
                hop.isDestination = true;
            } else if (reply->Status == IP_TTL_EXPIRED_TRANSIT ||
                       reply->Status == IP_TTL_EXPIRED_REASSEM) {
                // TTL expired - this is expected for intermediate hops
                hop.isDestination = false;
            }
        } else {
            // Timeout or error
            hop.address = "*";
            hop.rtt = -1;
        }

        emit hopResult(hop);

        if (hop.isDestination) {
            break;
        }
    }

    delete[] replyBuffer;
    IcmpCloseHandle(m_icmpHandle);
    m_icmpHandle = nullptr;

    emit finished(true, "Trace complete");

#else
    // Linux: Use UDP with ICMP error receiving
    // This approach uses UDP packets and listens for ICMP TTL exceeded messages

    int sendSock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sendSock < 0) {
        emit error("Failed to create UDP socket");
        emit finished(false, "Socket creation failed");
        return;
    }

    // Create ICMP socket for receiving TTL exceeded messages
    int recvSock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (recvSock < 0) {
        close(sendSock);
        // If we can't create raw socket, try fallback to system traceroute
        emit error("Native traceroute requires root privileges or CAP_NET_RAW capability.");
        emit error("Attempting to use system traceroute command...");

        // Use QProcess to run system traceroute
        QProcess process;
        process.start("traceroute", QStringList() << "-n" << "-m" << QString::number(m_maxHops)
                      << "-w" << QString::number(m_timeout / 1000) << m_targetAddress.toString());

        if (!process.waitForStarted(3000)) {
            emit finished(false, "Could not start system traceroute. Install with: sudo apt install traceroute");
            return;
        }

        int hopNum = 0;
        while (process.waitForReadyRead(m_timeout * 2) && m_running) {
            while (process.canReadLine() && m_running) {
                QString line = QString::fromUtf8(process.readLine()).trimmed();
                if (line.isEmpty() || line.startsWith("traceroute")) continue;

                // Parse traceroute output: "1  192.168.1.1  1.234 ms  1.456 ms  1.789 ms"
                QStringList parts = line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
                if (parts.size() >= 2) {
                    TraceHop hop;
                    hop.hop = parts[0].toInt();
                    if (hop.hop == 0) continue;

                    if (parts[1] == "*") {
                        hop.address = "*";
                        hop.rtt = -1;
                    } else {
                        hop.address = parts[1];
                        // Try to parse RTT
                        if (parts.size() >= 3) {
                            hop.rtt = static_cast<int>(parts[2].toDouble());
                        }
                        if (hop.address == m_targetAddress.toString()) {
                            hop.isDestination = true;
                        }
                    }
                    emit hopResult(hop);

                    if (hop.isDestination) {
                        process.terminate();
                        break;
                    }
                }
            }
        }

        process.waitForFinished(1000);
        emit finished(true, "Trace complete (via system command)");
        m_running = false;
        return;
    }

    // Set up destination address
    struct sockaddr_in destAddr;
    memset(&destAddr, 0, sizeof(destAddr));
    destAddr.sin_family = AF_INET;
    destAddr.sin_addr.s_addr = htonl(m_targetAddress.toIPv4Address());

    // Base port for UDP (traceroute typically uses 33434+)
    int basePort = 33434;

    for (int ttl = 1; ttl <= m_maxHops && m_running; ++ttl) {
        TraceHop hop;
        hop.hop = ttl;

        // Set TTL on the socket
        if (setsockopt(sendSock, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) < 0) {
            emit error(QString("Failed to set TTL to %1").arg(ttl));
            continue;
        }

        // Set destination port (increment for each probe)
        destAddr.sin_port = htons(basePort + ttl);

        QElapsedTimer timer;
        timer.start();

        // Send UDP packet
        char sendData[] = "RhenoCalc";
        if (sendto(sendSock, sendData, sizeof(sendData), 0,
                   (struct sockaddr*)&destAddr, sizeof(destAddr)) < 0) {
            emit error(QString("Failed to send packet for hop %1").arg(ttl));
            continue;
        }

        // Wait for ICMP response
        struct pollfd pfd;
        pfd.fd = recvSock;
        pfd.events = POLLIN;

        int pollResult = poll(&pfd, 1, m_timeout);

        if (pollResult > 0 && (pfd.revents & POLLIN)) {
            char recvBuffer[512];
            struct sockaddr_in fromAddr;
            socklen_t fromLen = sizeof(fromAddr);

            ssize_t recvLen = recvfrom(recvSock, recvBuffer, sizeof(recvBuffer), 0,
                                        (struct sockaddr*)&fromAddr, &fromLen);

            if (recvLen > 0) {
                hop.rtt = static_cast<int>(timer.elapsed());
                hop.address = QString::fromLatin1(inet_ntoa(fromAddr.sin_addr));

                // Parse ICMP header to check message type
                struct iphdr* ipHdr = (struct iphdr*)recvBuffer;
                int ipHdrLen = ipHdr->ihl * 4;
                struct icmphdr* icmpHdr = (struct icmphdr*)(recvBuffer + ipHdrLen);

                if (icmpHdr->type == ICMP_TIME_EXCEEDED) {
                    // Intermediate hop
                    hop.isDestination = false;
                } else if (icmpHdr->type == ICMP_DEST_UNREACH) {
                    // Destination reached (port unreachable is expected for UDP traceroute)
                    hop.isDestination = true;
                } else if (icmpHdr->type == ICMP_ECHOREPLY) {
                    hop.isDestination = true;
                }

                // Also check if we've reached the target by IP
                if (hop.address == m_targetAddress.toString()) {
                    hop.isDestination = true;
                }
            }
        } else {
            // Timeout
            hop.address = "*";
            hop.rtt = -1;
        }

        emit hopResult(hop);

        if (hop.isDestination) {
            break;
        }
    }

    close(sendSock);
    close(recvSock);

    emit finished(true, "Trace complete");
#endif

    m_running = false;
}

void TracerouteWorker::stop() {
    m_running = false;
}

// ─────────────────────────────────────────────────────────────────────────────
// Traceroute
// ─────────────────────────────────────────────────────────────────────────────

Traceroute::Traceroute(QObject* parent) : QObject(parent) {
}

Traceroute::~Traceroute() {
    stop();
}

void Traceroute::start(const QString& host, int maxHops, int timeout) {
    if (m_running) {
        stop();
    }

    m_running = true;
    m_thread = new QThread(this);
    m_worker = new TracerouteWorker(host, maxHops, timeout);
    m_worker->moveToThread(m_thread);

    connect(m_thread, &QThread::started, m_worker, &TracerouteWorker::start);
    connect(m_worker, &TracerouteWorker::hopResult, this, &Traceroute::hopResult);
    connect(m_worker, &TracerouteWorker::error, this, &Traceroute::error);
    connect(m_worker, &TracerouteWorker::finished, this, [this](bool success, const QString& message) {
        m_running = false;
        emit finished(success, message);
    });
    connect(m_worker, &TracerouteWorker::finished, m_thread, &QThread::quit);
    connect(m_thread, &QThread::finished, m_worker, &QObject::deleteLater);
    connect(m_thread, &QThread::finished, m_thread, &QObject::deleteLater);

    m_thread->start();
}

void Traceroute::stop() {
    if (m_worker) {
        m_worker->stop();
    }
    if (m_thread && m_thread->isRunning()) {
        m_thread->quit();
        m_thread->wait(1000);
    }
    m_running = false;
    m_worker = nullptr;
    m_thread = nullptr;
}

