#include "nettools.h"

#include <QStringList>

namespace Rheno::Core {

QList<int> parsePorts(const QString& portsExpression) {
    QList<int> ports;
    const QStringList parts = portsExpression.split(',', Qt::SkipEmptyParts);
    for (const QString& part : parts) {
        const QString p = part.trimmed();
        if (p.contains('-')) {
            const QStringList range = p.split('-');
            if (range.size() == 2) {
                const int start = range[0].toInt();
                const int end = range[1].toInt();
                for (int port = start; port <= end && port <= 65535; ++port) {
                    if (port > 0) ports.append(port);
                }
            }
        } else {
            const int port = p.toInt();
            if (port > 0 && port <= 65535) ports.append(port);
        }
    }
    return ports;
}

QString serviceNameForPort(int port) {
    switch (port) {
    case 21: return "FTP";
    case 22: return "SSH";
    case 23: return "Telnet";
    case 25: return "SMTP";
    case 53: return "DNS";
    case 80: return "HTTP";
    case 110: return "POP3";
    case 143: return "IMAP";
    case 443: return "HTTPS";
    case 445: return "SMB";
    case 3306: return "MySQL";
    case 3389: return "RDP";
    case 5432: return "PostgreSQL";
    case 8080: return "HTTP-Alt";
    default: return "";
    }
}

StyledLine classifyPingLine(const QString& line) {
    StyledLine out{line, ""};
    if (line.contains("Reply from") || line.contains("bytes from") || line.contains("64 bytes")) {
        out.color = "#2ecc71";
    } else if (line.contains("timed out") || line.contains("unreachable") ||
               line.contains("Destination") || line.contains("Request timed out")) {
        out.color = "#e74c3c";
    }
    return out;
}

StyledLine formatTraceHopLine(int hop, int rtt, const QString& address) {
    StyledLine out;
    if (rtt < 0) {
        out.text = QString("%1    *    Request timed out").arg(hop, 2);
        out.color = "#e74c3c";
    } else {
        out.text = QString("%1    %2 ms    %3").arg(hop, 2).arg(rtt, 4).arg(address);
        out.color = "#2ecc71";
    }
    return out;
}

} // namespace Rheno::Core


