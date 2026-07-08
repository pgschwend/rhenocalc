#include "floathandler.h"

#include <QtMath>

#include <cmath>
#include <limits>

namespace Rheno::Core {

FloatFormat getFloatFormat(const QString& typeName) {
    if (typeName == "float16 (Half)") return {16, 5, 10, 15};
    if (typeName == "float32 (Single)") return {32, 8, 23, 127};
    if (typeName == "float64 (Double)") return {64, 11, 52, 1023};
    return {32, 8, 23, 127};
}

QString doubleToIEEE(double value, int totalBits, int expBits, int mantBits) {
    const int bias = (1 << (expBits - 1)) - 1;
    if (std::isnan(value)) {
        QString result(totalBits, '0');
        for (int i = 1; i <= expBits; ++i) result[i] = '1';
        result[totalBits - 1] = '1';
        return result;
    }
    if (std::isinf(value)) {
        QString result(totalBits, '0');
        if (value < 0) result[0] = '1';
        for (int i = 1; i <= expBits; ++i) result[i] = '1';
        return result;
    }
    if (value == 0.0) {
        QString result(totalBits, '0');
        if (std::signbit(value)) result[0] = '1';
        return result;
    }

    QString result(totalBits, '0');
    if (value < 0) {
        result[0] = '1';
        value = -value;
    }

    int exponent = 0;
    double mantissa = std::frexp(value, &exponent);
    mantissa *= 2.0;
    exponent -= 1;
    int biasedExp = exponent + bias;

    if (biasedExp >= (1 << expBits) - 1) {
        for (int i = 1; i <= expBits; ++i) result[i] = '1';
        return result;
    }
    if (biasedExp <= 0) {
        biasedExp = 0;
        mantissa = value / std::pow(2.0, 1 - bias);
    } else {
        mantissa -= 1.0;
    }

    for (int i = expBits - 1; i >= 0; --i)
        result[1 + (expBits - 1 - i)] = (biasedExp & (1 << i)) ? '1' : '0';

    for (int i = 0; i < mantBits; ++i) {
        mantissa *= 2.0;
        if (mantissa >= 1.0) {
            result[1 + expBits + i] = '1';
            mantissa -= 1.0;
        }
    }
    return result;
}

double ieeeToDouble(const QString& bits, int totalBits, int expBits, int mantBits) {
    if (bits.length() != totalBits) return std::numeric_limits<double>::quiet_NaN();
    const int bias = (1 << (expBits - 1)) - 1;
    const int sign = (bits[0] == '1') ? -1 : 1;

    int exponent = 0;
    for (int i = 1; i <= expBits; ++i)
        exponent = (exponent << 1) | (bits[i] == '1' ? 1 : 0);

    double mantissa = 0.0;
    double fraction = 0.5;
    for (int i = 1 + expBits; i < totalBits; ++i) {
        if (bits[i] == '1') mantissa += fraction;
        fraction /= 2.0;
    }

    const int maxExp = (1 << expBits) - 1;
    if (exponent == 0) {
        if (mantissa == 0.0) return sign > 0 ? 0.0 : -0.0;
        return sign * mantissa * std::pow(2.0, 1 - bias);
    }
    if (exponent == maxExp) {
        if (mantissa == 0.0) return sign > 0 ? std::numeric_limits<double>::infinity() : -std::numeric_limits<double>::infinity();
        return std::numeric_limits<double>::quiet_NaN();
    }

    mantissa += 1.0;
    return sign * mantissa * std::pow(2.0, exponent - bias);
}

QString formatBinaryString(const QString& bits, int expBits, int mantBits) {
    if (bits.length() != 1 + expBits + mantBits) return bits;
    return bits.left(1) + " " + bits.mid(1, expBits) + " " + bits.mid(1 + expBits);
}

static QString bitsToHex(const QString& bits) {
    QString hex;
    for (int i = 0; i < bits.length(); i += 4)
        hex += QString::number(bits.mid(i, 4).toInt(nullptr, 2), 16).toUpper();
    return "0x" + hex;
}

FloatEncodeResult encodeFloatText(const QString& text, const QString& typeName) {
    FloatEncodeResult out;
    QString t = text.trimmed();
    if (t.isEmpty()) {
        out.error = "empty";
        return out;
    }
    bool ok = false;
    double value = t.toDouble(&ok);
    if (!ok) {
        t.replace(',', '.');
        value = t.toDouble(&ok);
    }
    if (!ok) {
        out.error = "invalid";
        return out;
    }

    const FloatFormat fmt = getFloatFormat(typeName);
    const QString binary = doubleToIEEE(value, fmt.totalBits, fmt.expBits, fmt.mantBits);
    out.sign = binary.left(1);
    out.exponent = binary.mid(1, fmt.expBits);
    out.mantissa = binary.mid(1 + fmt.expBits);
    out.binarySpaced = formatBinaryString(binary, fmt.expBits, fmt.mantBits);
    out.hex = bitsToHex(binary);
    out.valid = true;
    return out;
}

FloatDecodeResult decodeFloatBits(QString bits, const QString& typeName) {
    FloatDecodeResult out;
    bits.remove(' ').remove('-').remove('_');
    const FloatFormat fmt = getFloatFormat(typeName);
    if (bits.length() != fmt.totalBits) {
        out.error = QString("Need %1 bits").arg(fmt.totalBits);
        return out;
    }
    for (const QChar c : bits) {
        if (c != '0' && c != '1') {
            out.error = "Invalid binary";
            return out;
        }
    }

    out.sign = bits.left(1);
    out.exponent = bits.mid(1, fmt.expBits);
    out.mantissa = bits.mid(1 + fmt.expBits);
    const double value = ieeeToDouble(bits, fmt.totalBits, fmt.expBits, fmt.mantBits);
    if (std::isnan(value)) out.decimal = "NaN";
    else if (std::isinf(value)) out.decimal = value > 0 ? "+∞" : "−∞";
    else {
        const int precision = (fmt.totalBits == 16) ? 4 : (fmt.totalBits == 32) ? 9 : 17;
        out.decimal = QString::number(value, 'g', precision);
    }
    out.hex = bitsToHex(bits);
    out.valid = true;
    return out;
}

} // namespace Rheno::Core

