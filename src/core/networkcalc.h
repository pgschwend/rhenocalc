#pragma once

#include <QString>
#include <QtGlobal>

namespace NetworkCalc {

struct SubnetResult {
    bool valid = false;
    QString error;

    quint32 ip = 0;
    quint32 mask = 0;
    quint32 wildcard = 0;
    quint32 network = 0;
    quint32 broadcast = 0;
    quint32 firstHost = 0;
    quint32 lastHost = 0;

    int prefix = 0;
    quint64 addressCount = 0;
    quint64 usableHosts = 0;
};

struct IpInfo {
    QString ipClass;
    QString scope;
    bool isPrivate = false;
    bool isLoopback = false;
    bool isLinkLocal = false;
    bool isMulticast = false;
    bool isBroadcast = false;
};

bool parseIpv4(const QString& text, quint32& outIp, QString* error = nullptr);
QString toIpv4(quint32 ip);

quint32 prefixToMask(int prefix);
bool maskToPrefix(quint32 mask, int& outPrefix, QString* error = nullptr);
bool parseCidr(const QString& text, quint32& outIp, int& outPrefix, QString* error = nullptr);

SubnetResult calculateSubnet(quint32 ip, int prefix);
quint64 usableHostsForPrefix(int prefix);
bool minimalPrefixForHosts(quint64 devices, int& outPrefix, QString* error = nullptr);

IpInfo classifyIp(quint32 ip);

} // namespace NetworkCalc

