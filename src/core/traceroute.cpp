#include "traceroute.h"

#include <QHostInfo>
#include <QElapsedTimer>

#ifdef Q_OS_WIN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <icmpapi.h>
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")
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
    // Linux/Mac: Use system traceroute (ICMP requires root)
    emit error("Native traceroute not available on this platform. Using system command.");
    emit finished(false, "Platform not supported for native traceroute");
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

