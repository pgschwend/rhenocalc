#include "crchashcore.h"

#include <QCryptographicHash>

namespace {

struct CrcSpec {
    int width;
    quint64 poly;
    quint64 init;
    quint64 xorOut;
    bool refin;
    bool refout;
};

quint64 reflectBits(quint64 value, int bits) {
    quint64 out = 0;
    for (int i = 0; i < bits; ++i) {
        if (value & (1ULL << i))
            out |= (1ULL << (bits - 1 - i));
    }
    return out;
}

quint64 crcCompute(const QByteArray& data, const CrcSpec& spec) {
    const quint64 topBit = 1ULL << (spec.width - 1);
    const quint64 mask = ((1ULL << spec.width) - 1ULL);

    quint64 crc = spec.init & mask;
    for (unsigned char byte : data) {
        quint64 cur = byte;
        if (spec.refin)
            cur = reflectBits(cur, 8);

        crc ^= (cur << (spec.width - 8));
        for (int i = 0; i < 8; ++i) {
            if (crc & topBit)
                crc = (crc << 1) ^ spec.poly;
            else
                crc <<= 1;
            crc &= mask;
        }
    }

    if (spec.refout)
        crc = reflectBits(crc, spec.width);

    crc ^= spec.xorOut;
    return crc & mask;
}

quint32 rotl32(quint32 v, int r) {
    return (v << r) | (v >> (32 - r));
}

quint32 fmix32(quint32 h) {
    h ^= h >> 16;
    h *= 0x85ebca6bU;
    h ^= h >> 13;
    h *= 0xc2b2ae35U;
    h ^= h >> 16;
    return h;
}

quint32 murmurHash3_x86_32(const QByteArray& data, quint32 seed = 0) {
    const auto* bytes = reinterpret_cast<const unsigned char*>(data.constData());
    const int len = static_cast<int>(data.size());
    const int nblocks = len / 4;

    quint32 h1 = seed;
    constexpr quint32 c1 = 0xcc9e2d51U;
    constexpr quint32 c2 = 0x1b873593U;

    for (int i = 0; i < nblocks; ++i) {
        const int idx = i * 4;
        quint32 k1 = static_cast<quint32>(bytes[idx])
            | (static_cast<quint32>(bytes[idx + 1]) << 8)
            | (static_cast<quint32>(bytes[idx + 2]) << 16)
            | (static_cast<quint32>(bytes[idx + 3]) << 24);

        k1 *= c1;
        k1 = rotl32(k1, 15);
        k1 *= c2;

        h1 ^= k1;
        h1 = rotl32(h1, 13);
        h1 = h1 * 5U + 0xe6546b64U;
    }

    quint32 k1 = 0;
    const unsigned char* tail = bytes + (nblocks * 4);
    switch (len & 3) {
    case 3:
        k1 ^= static_cast<quint32>(tail[2]) << 16;
        [[fallthrough]];
    case 2:
        k1 ^= static_cast<quint32>(tail[1]) << 8;
        [[fallthrough]];
    case 1:
        k1 ^= static_cast<quint32>(tail[0]);
        k1 *= c1;
        k1 = rotl32(k1, 15);
        k1 *= c2;
        h1 ^= k1;
        break;
    default:
        break;
    }

    h1 ^= static_cast<quint32>(len);
    return fmix32(h1);
}

quint32 xxHash32(const QByteArray& data, quint32 seed = 0) {
    const auto* p = reinterpret_cast<const unsigned char*>(data.constData());
    int len = static_cast<int>(data.size());
    const unsigned char* end = p + len;

    constexpr quint32 p1 = 2654435761U;
    constexpr quint32 p2 = 2246822519U;
    constexpr quint32 p3 = 3266489917U;
    constexpr quint32 p4 = 668265263U;
    constexpr quint32 p5 = 374761393U;

    auto read32 = [](const unsigned char* ptr) {
        return static_cast<quint32>(ptr[0])
            | (static_cast<quint32>(ptr[1]) << 8)
            | (static_cast<quint32>(ptr[2]) << 16)
            | (static_cast<quint32>(ptr[3]) << 24);
    };

    quint32 h32 = 0;
    if (len >= 16) {
        quint32 v1 = seed + p1 + p2;
        quint32 v2 = seed + p2;
        quint32 v3 = seed;
        quint32 v4 = seed - p1;

        const unsigned char* limit = end - 16;
        do {
            v1 += read32(p) * p2;
            v1 = rotl32(v1, 13);
            v1 *= p1;
            p += 4;

            v2 += read32(p) * p2;
            v2 = rotl32(v2, 13);
            v2 *= p1;
            p += 4;

            v3 += read32(p) * p2;
            v3 = rotl32(v3, 13);
            v3 *= p1;
            p += 4;

            v4 += read32(p) * p2;
            v4 = rotl32(v4, 13);
            v4 *= p1;
            p += 4;
        } while (p <= limit);

        h32 = rotl32(v1, 1) + rotl32(v2, 7) + rotl32(v3, 12) + rotl32(v4, 18);
    } else {
        h32 = seed + p5;
    }

    h32 += static_cast<quint32>(len);

    while (p + 4 <= end) {
        h32 += read32(p) * p3;
        h32 = rotl32(h32, 17) * p4;
        p += 4;
    }

    while (p < end) {
        h32 += (*p) * p5;
        h32 = rotl32(h32, 11) * p1;
        ++p;
    }

    h32 ^= h32 >> 15;
    h32 *= p2;
    h32 ^= h32 >> 13;
    h32 *= p3;
    h32 ^= h32 >> 16;

    return h32;
}

QString toHex(quint64 value, int widthHex) {
    return QString("0x%1").arg(QString::number(value, 16).toUpper().rightJustified(widthHex, '0'));
}

QString crcHex(const QByteArray& data, const CrcSpec& spec) {
    const int digits = (spec.width + 3) / 4;
    return toHex(crcCompute(data, spec), digits);
}

QString hashHex(const QByteArray& data, QCryptographicHash::Algorithm alg) {
    return QCryptographicHash::hash(data, alg).toHex().toUpper();
}

QString algorithmFormula(CrcHashCore::Algorithm alg) {
    switch (alg) {
    case CrcHashCore::Algorithm::Crc8Maxim:
        return "CRC-8/MAXIM: G(x)=x^8+x^5+x^4+1, init=0x00, xorOut=0x00, refin/refout=true.";
    case CrcHashCore::Algorithm::Crc8J1850:
        return "CRC-8/J1850: G(x)=x^8+x^4+x^3+x^2+1, init=0xFF, xorOut=0xFF, refin/refout=false.";
    case CrcHashCore::Algorithm::Crc16Modbus:
        return "CRC-16/MODBUS: G(x)=x^16+x^15+x^2+1, init=0xFFFF, xorOut=0x0000, refin/refout=true.";
    case CrcHashCore::Algorithm::CrcCcitt:
        return "CRC-CCITT: G(x)=x^16+x^12+x^5+1, init=0xFFFF, xorOut=0x0000, refin/refout=false.";
    case CrcHashCore::Algorithm::Crc32Iso3309:
        return "CRC-32/ISO 3309: G(x)=0x04C11DB7, init=0xFFFFFFFF, xorOut=0xFFFFFFFF, refin/refout=true.";
    case CrcHashCore::Algorithm::Crc32C:
        return "CRC-32C/Castagnoli: G(x)=0x1EDC6F41, init=0xFFFFFFFF, xorOut=0xFFFFFFFF, refin/refout=true.";
    case CrcHashCore::Algorithm::MurmurHash3:
        return "MurmurHash3 x86_32: k*=c1; k=ROTL32(k,15); k*=c2; h^=k; h=ROTL32(h,13)*5+0xe6546b64; h=fmix32(h^len).";
    case CrcHashCore::Algorithm::XxHash32:
        return "xxHash32: block mixing with primes p1..p5, avalanche steps h^=h>>15; h*=p2; h^=h>>13; h*=p3; h^=h>>16.";
    case CrcHashCore::Algorithm::Md5:
        return "MD5: digest = MD5(m), 128-bit Merkle-Damgard hash over 512-bit blocks.";
    case CrcHashCore::Algorithm::Sha1:
        return "SHA-1: digest = SHA1(m), 160-bit Merkle-Damgard hash over 512-bit blocks.";
    case CrcHashCore::Algorithm::Sha256:
        return "SHA-256: digest = SHA256(m), 256-bit SHA-2 compression over 512-bit blocks.";
    case CrcHashCore::Algorithm::Sha512:
        return "SHA-512: digest = SHA512(m), 512-bit SHA-2 compression over 1024-bit blocks.";
    }

    return {};
}

} // namespace

namespace CrcHashCore {

QVector<AlgorithmEntry> algorithms() {
    return {
        {"CRC-8 MAXIM", Algorithm::Crc8Maxim},
        {"CRC-8 J1850", Algorithm::Crc8J1850},
        {"CRC-16 Modbus", Algorithm::Crc16Modbus},
        {"CRC-CCITT", Algorithm::CrcCcitt},
        {"CRC-32 ISO 3309", Algorithm::Crc32Iso3309},
        {"CRC-32C (Castagnoli)", Algorithm::Crc32C},
        {"MurmurHash3 (x86_32)", Algorithm::MurmurHash3},
        {"xxHash32", Algorithm::XxHash32},
        {"MD5", Algorithm::Md5},
        {"SHA-1", Algorithm::Sha1},
        {"SHA-256", Algorithm::Sha256},
        {"SHA-512", Algorithm::Sha512},
    };
}

ComputeResult compute(Algorithm algorithm, const QByteArray& input) {
    ComputeResult out;
    out.formula = algorithmFormula(algorithm);

    switch (algorithm) {
    case Algorithm::Crc8Maxim:
        out.value = crcHex(input, {8, 0x31, 0x00, 0x00, true, true});
        break;
    case Algorithm::Crc8J1850:
        out.value = crcHex(input, {8, 0x1D, 0xFF, 0xFF, false, false});
        break;
    case Algorithm::Crc16Modbus:
        out.value = crcHex(input, {16, 0x8005, 0xFFFF, 0x0000, true, true});
        break;
    case Algorithm::CrcCcitt:
        out.value = crcHex(input, {16, 0x1021, 0xFFFF, 0x0000, false, false});
        break;
    case Algorithm::Crc32Iso3309:
        out.value = crcHex(input, {32, 0x04C11DB7, 0xFFFFFFFF, 0xFFFFFFFF, true, true});
        break;
    case Algorithm::Crc32C:
        out.value = crcHex(input, {32, 0x1EDC6F41, 0xFFFFFFFF, 0xFFFFFFFF, true, true});
        break;
    case Algorithm::MurmurHash3:
        out.value = toHex(murmurHash3_x86_32(input, 0), 8);
        break;
    case Algorithm::XxHash32:
        out.value = toHex(xxHash32(input, 0), 8);
        break;
    case Algorithm::Md5:
        out.value = hashHex(input, QCryptographicHash::Md5);
        break;
    case Algorithm::Sha1:
        out.value = hashHex(input, QCryptographicHash::Sha1);
        break;
    case Algorithm::Sha256:
        out.value = hashHex(input, QCryptographicHash::Sha256);
        break;
    case Algorithm::Sha512:
        out.value = hashHex(input, QCryptographicHash::Sha512);
        break;
    }

    return out;
}

} // namespace CrcHashCore

