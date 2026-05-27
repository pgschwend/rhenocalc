#pragma once

#include <QByteArray>
#include <QString>
#include <QVector>

namespace CrcHashCore {

enum class Algorithm {
    Crc8Maxim,
    Crc8J1850,
    Crc16Modbus,
    CrcCcitt,
    Crc32Iso3309,
    Crc32C,
    MurmurHash3,
    XxHash32,
    Md5,
    Sha1,
    Sha256,
    Sha512,
};

struct AlgorithmEntry {
    QString name;
    Algorithm algorithm;
};

struct ComputeResult {
    QString value;
    QString formula;
};

QVector<AlgorithmEntry> algorithms();
ComputeResult compute(Algorithm algorithm, const QByteArray& input);

} // namespace CrcHashCore

