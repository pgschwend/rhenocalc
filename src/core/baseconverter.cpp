#include "baseconverter.h"

#include <cmath>
#include <cstring>

namespace Rheno::Core {

unsigned long long applyMask(unsigned long long value, int bits) {
    if (bits == 64) return value;
    return value & ((1ULL << bits) - 1);
}

QString formatBinarySpaced(unsigned long long value, int bits) {
    QString b = QString::number(value, 2);
    while (b.length() < bits) b.prepend('0');

    QString spaced;
    for (int i = 0; i < b.length(); ++i) {
        if (i > 0 && (b.length() - i) % 4 == 0)
            spaced += ' ';
        spaced += b[i];
    }
    return spaced;
}

bool tryParse(const QString& text, int base, unsigned long long& outValue) {
    const QString s = text.trimmed().remove(' ');
    bool ok = false;

    outValue = s.toULongLong(&ok, base);
    if (ok)
        return true;

    if (base == 10) {
        const long long signedValue = s.toLongLong(&ok, 10);
        if (ok) {
            outValue = static_cast<unsigned long long>(signedValue);
            return true;
        }
    }

    return false;
}

long long signedValue(unsigned long long value, int bits) {
    long long sval = static_cast<long long>(value);
    if (bits < 64) {
        const long long signBit = 1LL << (bits - 1);
        if (value & static_cast<unsigned long long>(signBit))
            sval = static_cast<long long>(value | ~((1ULL << bits) - 1));
    }
    return sval;
}

QString hexWithPadding(unsigned long long value, int bits) {
    return "0x" + QString::number(value, 16).toUpper().rightJustified(bits / 4, '0');
}

QString float32String(unsigned long long value) {
    quint32 v32 = static_cast<quint32>(value);
    float f = 0.0f;
    std::memcpy(&f, &v32, 4);

    if (std::isnan(f)) return "NaN";
    if (std::isinf(f)) return f > 0 ? "+Inf" : "-Inf";
    return QString::number(static_cast<double>(f), 'g', 8);
}

QString byteHex(unsigned long long value, int byteIndex) {
    const quint8 byte = static_cast<quint8>((value >> (byteIndex * 8)) & 0xFF);
    return QString("%1").arg(byte, 2, 16, QChar('0')).toUpper();
}

QString byteTooltip(unsigned long long value, int byteIndex) {
    const quint8 byte = static_cast<quint8>((value >> (byteIndex * 8)) & 0xFF);
    return QString("Byte %1: 0x%2 = %3")
        .arg(byteIndex)
        .arg(byte, 2, 16, QChar('0'))
        .toUpper()
        .arg(byte);
}

} // namespace Rheno::Core

