#pragma once

#include <QString>

namespace Rheno::Core {

struct FloatFormat {
    int totalBits;
    int expBits;
    int mantBits;
    int bias;
};

struct FloatEncodeResult {
    bool valid = false;
    QString sign;
    QString exponent;
    QString mantissa;
    QString binarySpaced;
    QString hex;
    QString error;
};

struct FloatDecodeResult {
    bool valid = false;
    QString sign;
    QString exponent;
    QString mantissa;
    QString decimal;
    QString hex;
    QString error;
};

FloatFormat getFloatFormat(const QString& typeName);
QString doubleToIEEE(double value, int totalBits, int expBits, int mantBits);
double ieeeToDouble(const QString& bits, int totalBits, int expBits, int mantBits);
QString formatBinaryString(const QString& bits, int expBits, int mantBits);

FloatEncodeResult encodeFloatText(const QString& text, const QString& typeName);
FloatDecodeResult decodeFloatBits(QString bits, const QString& typeName);

} // namespace Rheno::Core

