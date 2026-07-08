#pragma once

#include <QString>
#include <boost/multiprecision/cpp_dec_float.hpp>

namespace Rheno::Core {

using BigDecimal = boost::multiprecision::cpp_dec_float_50;

long long maskToWidth(long long value, int bits);
QString toBaseString(long long value, int base, int bits);
long long fromBaseString(const QString& text, int base);
QString formatDouble(double value);
QString formatBigDecimal(const BigDecimal& value);

long long applyBinary(long long a, long long b, const QString& op);
double applyBinary(double a, double b, const QString& op);

long long applyUnaryInt(long long value, const QString& op, int bits);
double applyUnaryDouble(double value, const QString& op);

BigDecimal applyBigBinary(const BigDecimal& a, const BigDecimal& b, const QString& op);
BigDecimal applyBigUnary(const BigDecimal& value, const QString& op);

} // namespace Rheno::Core
