#pragma once

#include <QString>

namespace CalculatorCore {

long long maskToWidth(long long value, int bits);
QString toBaseString(long long value, int base, int bits);
long long fromBaseString(const QString& text, int base);
QString formatDouble(double value);

long long applyBinary(long long a, long long b, const QString& op);
double applyBinary(double a, double b, const QString& op);

long long applyUnaryInt(long long value, const QString& op, int bits);
double applyUnaryDouble(double value, const QString& op);

} // namespace CalculatorCore

