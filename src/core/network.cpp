#include "network.h"

#include <QStringList>

namespace {

quint32 octetsToIp(int a, int b, int c, int d) {
    return (static_cast<quint32>(a) << 24)
        | (static_cast<quint32>(b) << 16)
        | (static_cast<quint32>(c) << 8)
        | static_cast<quint32>(d);
}

bool inRange(quint32 ip, quint32 from, quint32 to) {
    return ip >= from && ip <= to;
}

} // namespace

namespace Rheno::Core {

bool parseIpv4(const QString& text, quint32& outIp, QString* error) {
    const QString t = text.trimmed();
    const QStringList parts = t.split('.');
    if (parts.size() != 4) {
        if (error) *error = "IPv4 address must have 4 octets.";
        return false;
    }

    int oct[4] = {0, 0, 0, 0};
    for (int i = 0; i < 4; ++i) {
        bool ok = false;
        const int v = parts[i].toInt(&ok);
        if (!ok || v < 0 || v > 255) {
            if (error) *error = QString("Invalid octet %1: '%2'.").arg(i + 1).arg(parts[i]);
            return false;
        }
        oct[i] = v;
    }

    outIp = octetsToIp(oct[0], oct[1], oct[2], oct[3]);
    return true;
}

QString toIpv4(quint32 ip) {
    return QString("%1.%2.%3.%4")
        .arg((ip >> 24) & 0xFF)
        .arg((ip >> 16) & 0xFF)
        .arg((ip >> 8) & 0xFF)
        .arg(ip & 0xFF);
}

quint32 prefixToMask(int prefix) {
    if (prefix <= 0) return 0;
    if (prefix >= 32) return 0xFFFFFFFFu;
    return 0xFFFFFFFFu << (32 - prefix);
}

bool maskToPrefix(quint32 mask, int& outPrefix, QString* error) {
    int prefix = 0;
    bool zeroSeen = false;
    for (int i = 31; i >= 0; --i) {
        const bool bit = ((mask >> i) & 1u) != 0u;
        if (bit && zeroSeen) {
            if (error) *error = "Subnet mask is not contiguous.";
            return false;
        }
        if (bit) {
            ++prefix;
        } else {
            zeroSeen = true;
        }
    }
    outPrefix = prefix;
    return true;
}

bool parseCidr(const QString& text, quint32& outIp, int& outPrefix, QString* error) {
    const QString t = text.trimmed();
    const int slash = t.indexOf('/');
    if (slash <= 0 || slash == t.size() - 1) {
        if (error) *error = "CIDR must look like 192.168.1.10/24.";
        return false;
    }

    const QString ipPart = t.left(slash).trimmed();
    const QString suffixPart = t.mid(slash + 1).trimmed();

    if (!parseIpv4(ipPart, outIp, error))
        return false;

    bool okPrefix = false;
    const int prefix = suffixPart.toInt(&okPrefix);
    if (okPrefix) {
        if (prefix < 0 || prefix > 32) {
            if (error) *error = "Prefix must be between 0 and 32.";
            return false;
        }
        outPrefix = prefix;
        return true;
    }

    quint32 mask = 0;
    if (!parseIpv4(suffixPart, mask, error)) {
        if (error && error->isEmpty())
            *error = "CIDR suffix must be prefix length or dotted mask.";
        return false;
    }

    return maskToPrefix(mask, outPrefix, error);
}

SubnetResult calculateSubnet(quint32 ip, int prefix) {
    SubnetResult r;
    if (prefix < 0 || prefix > 32) {
        r.error = "Prefix must be between 0 and 32.";
        return r;
    }

    r.valid = true;
    r.ip = ip;
    r.prefix = prefix;
    r.mask = prefixToMask(prefix);
    r.wildcard = ~r.mask;
    r.network = ip & r.mask;
    r.broadcast = r.network | r.wildcard;

    if (prefix == 32) {
        r.firstHost = ip;
        r.lastHost = ip;
        r.addressCount = 1;
        r.usableHosts = 1;
        return r;
    }

    if (prefix == 31) {
        r.firstHost = r.network;
        r.lastHost = r.broadcast;
        r.addressCount = 2;
        r.usableHosts = 2;
        return r;
    }

    r.firstHost = r.network + 1;
    r.lastHost = r.broadcast - 1;
    r.addressCount = (1ULL << (32 - prefix));
    r.usableHosts = r.addressCount >= 2 ? r.addressCount - 2 : 0;
    return r;
}

quint64 usableHostsForPrefix(int prefix) {
    if (prefix < 0 || prefix > 32)
        return 0;
    if (prefix == 32)
        return 1;
    if (prefix == 31)
        return 2;
    return (1ULL << (32 - prefix)) - 2ULL;
}

bool minimalPrefixForHosts(quint64 devices, int& outPrefix, QString* error) {
    if (devices == 0) {
        if (error) *error = "Device count must be >= 1.";
        return false;
    }

    for (int prefix = 32; prefix >= 0; --prefix) {
        if (usableHostsForPrefix(prefix) >= devices) {
            outPrefix = prefix;
            return true;
        }
    }

    if (error) *error = "Device count is too large for IPv4.";
    return false;
}

IpInfo classifyIp(quint32 ip) {
    IpInfo info;
    const quint8 a = static_cast<quint8>((ip >> 24) & 0xFFu);

    if (a <= 127) info.ipClass = "Class A";
    else if (a <= 191) info.ipClass = "Class B";
    else if (a <= 223) info.ipClass = "Class C";
    else if (a <= 239) info.ipClass = "Class D";
    else info.ipClass = "Class E";

    info.isLoopback = inRange(ip, octetsToIp(127, 0, 0, 0), octetsToIp(127, 255, 255, 255));
    info.isLinkLocal = inRange(ip, octetsToIp(169, 254, 0, 0), octetsToIp(169, 254, 255, 255));
    info.isMulticast = inRange(ip, octetsToIp(224, 0, 0, 0), octetsToIp(239, 255, 255, 255));
    info.isBroadcast = (ip == 0xFFFFFFFFu);

    const bool privateA = inRange(ip, octetsToIp(10, 0, 0, 0), octetsToIp(10, 255, 255, 255));
    const bool privateB = inRange(ip, octetsToIp(172, 16, 0, 0), octetsToIp(172, 31, 255, 255));
    const bool privateC = inRange(ip, octetsToIp(192, 168, 0, 0), octetsToIp(192, 168, 255, 255));
    info.isPrivate = privateA || privateB || privateC;

    if (info.isLoopback) info.scope = "Loopback";
    else if (info.isLinkLocal) info.scope = "Link-local";
    else if (info.isMulticast) info.scope = "Multicast";
    else if (info.isBroadcast) info.scope = "Limited broadcast";
    else if (info.isPrivate) info.scope = "Private";
    else info.scope = "Public";

    return info;
}

} // namespace Rheno::Core

