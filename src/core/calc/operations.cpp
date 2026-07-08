#include "operations.h"

#include <cmath>
#include <limits>

#include <boost/multiprecision/cpp_dec_float.hpp>

namespace Rheno::Core {

long long maskToWidth(long long value, int bits) {
    if (bits == 64) return value;
    const long long mask = (1LL << bits) - 1;
    return value & mask;
}

QString toBaseString(long long value, int base, int bits) {
    const long long masked = maskToWidth(value, bits);
    if (base == 16) return QString::number(static_cast<unsigned long long>(masked), 16).toUpper();
    if (base == 2) return QString::number(static_cast<unsigned long long>(masked), 2);
    if (base == 8) return QString::number(static_cast<unsigned long long>(masked), 8);
    return QString::number(masked);
}

long long fromBaseString(const QString& text, int base) {
    bool ok = false;
    const long long v = text.toLongLong(&ok, base);
    return ok ? v : 0;
}

QString formatDouble(double value) {
    if (std::isinf(value)) return value > 0 ? "∞" : "-∞";
    if (std::isnan(value)) return "NaN";
    return QString::number(value, 'g', 12);
}

QString formatBigDecimal(const BigDecimal& value) {
    constexpr auto digits = std::numeric_limits<BigDecimal>::max_digits10;
    std::string s = value.str(digits);
    if (s.empty() || s == "-0") s = "0";
    return QString::fromStdString(s);
}

// ── Integer arithmetic ───────────────────────────────────────────────────────

long long applyBinary(long long a, long long b, const QString& op) {
    if (op == "+") return a + b;
    if (op == "-") return a - b;
    if (op == "*") return a * b;
    if (op == "/") return b != 0 ? a / b : 0;
    if (op == "MOD") return b != 0 ? a % b : 0;
    if (op == "AND") return a & b;
    if (op == "OR") return a | b;
    if (op == "XOR") return a ^ b;
    return a;
}

long long applyUnaryInt(long long value, const QString& op, int bits) {
    if (op == "SQ") return maskToWidth(value * value, bits);
    if (op == "SQRT") return value >= 0 ? static_cast<long long>(std::sqrt(static_cast<double>(value))) : 0;
    if (op == "NOT") return maskToWidth(~value, bits);
    if (op == "LSL") return maskToWidth(value << 1, bits);
    if (op == "LSR") return maskToWidth(static_cast<long long>(static_cast<unsigned long long>(value) >> 1), bits);
    if (op == "ROL") {
        const auto v = static_cast<unsigned long long>(maskToWidth(value, bits));
        return maskToWidth(static_cast<long long>((v << 1) | (v >> (bits - 1))), bits);
    }
    if (op == "ROR") {
        const auto v = static_cast<unsigned long long>(maskToWidth(value, bits));
        return maskToWidth(static_cast<long long>((v >> 1) | (v << (bits - 1))), bits);
    }
    return value;
}

// ── Double arithmetic ────────────────────────────────────────────────────────

double applyBinary(double a, double b, const QString& op) {
    if (op == "+") return a + b;
    if (op == "-") return a - b;
    if (op == "*") return a * b;
    if (op == "/") return b != 0.0 ? a / b : std::numeric_limits<double>::infinity();
    if (op == "MOD") return std::fmod(a, b);
    return a;
}

double applyUnaryDouble(double value, const QString& op) {
    if (op == "SQ") return value * value;
    if (op == "SQRT") return value >= 0.0 ? std::sqrt(value) : std::numeric_limits<double>::quiet_NaN();
    if (op == "1/x") return value != 0.0 ? 1.0 / value : std::numeric_limits<double>::infinity();
    if (op == "log") return value > 0.0 ? std::log10(value) : std::numeric_limits<double>::quiet_NaN();
    if (op == "ln") return value > 0.0 ? std::log(value) : std::numeric_limits<double>::quiet_NaN();
    // Trigonometric functions (input in DEGREES)
    constexpr double degToRad = 3.14159265358979323846 / 180.0;
    constexpr double radToDeg = 180.0 / 3.14159265358979323846;
    if (op == "sin") return std::sin(value * degToRad);
    if (op == "cos") return std::cos(value * degToRad);
    if (op == "tan") return std::tan(value * degToRad);
    // Inverse trig functions (output in DEGREES)
    if (op == "asin") return (value >= -1.0 && value <= 1.0) ? std::asin(value) * radToDeg : std::numeric_limits<double>::quiet_NaN();
    if (op == "acos") return (value >= -1.0 && value <= 1.0) ? std::acos(value) * radToDeg : std::numeric_limits<double>::quiet_NaN();
    if (op == "atan") return std::atan(value) * radToDeg;
    // Hyperbolic functions
    if (op == "sinh") return std::sinh(value);
    if (op == "cosh") return std::cosh(value);
    if (op == "tanh") return std::tanh(value);
    return value;
}

// ── BigDecimal arithmetic ────────────────────────────────────────────────────

BigDecimal applyBigBinary(const BigDecimal& a, const BigDecimal& b, const QString& op) {
    if (op == "+") return a + b;
    if (op == "-") return a - b;
    if (op == "*") return a * b;
    if (op == "/") return b != 0 ? a / b : BigDecimal(0);
    if (op == "MOD") {
        if (b == 0) return BigDecimal(0);
        BigDecimal q = a / b;
        q = boost::multiprecision::trunc(q);
        return a - q * b;
    }
    // Bitwise: fallback to long long
    long long la = static_cast<long long>(a);
    long long lb = static_cast<long long>(b);
    if (op == "AND") return BigDecimal(la & lb);
    if (op == "OR")  return BigDecimal(la | lb);
    if (op == "XOR") return BigDecimal(la ^ lb);
    return a;
}

BigDecimal applyBigUnary(const BigDecimal& value, const QString& op) {
    if (op == "SQ")   return value * value;
    if (op == "SQRT") return value >= 0 ? boost::multiprecision::sqrt(value) : BigDecimal(0);
    if (op == "1/x")  return value != 0 ? BigDecimal(1) / value : BigDecimal(0);
    if (op == "log")  return value > 0 ? boost::multiprecision::log10(value) : BigDecimal(0);
    if (op == "ln")   return value > 0 ? boost::multiprecision::log(value) : BigDecimal(0);
    if (op == "NOT") {
        long long v = static_cast<long long>(value);
        return BigDecimal(~v);
    }
    if (op == "LSL") return value * 2;
    if (op == "LSR") return boost::multiprecision::trunc(value / 2);
    // Trigonometric functions (DEGREES) - convert to double for calculation
    constexpr double degToRad = 3.14159265358979323846 / 180.0;
    constexpr double radToDeg = 180.0 / 3.14159265358979323846;
    if (op == "sin") return BigDecimal(std::sin(static_cast<double>(value) * degToRad));
    if (op == "cos") return BigDecimal(std::cos(static_cast<double>(value) * degToRad));
    if (op == "tan") return BigDecimal(std::tan(static_cast<double>(value) * degToRad));
    if (op == "asin") {
        double d = static_cast<double>(value);
        return (d >= -1.0 && d <= 1.0) ? BigDecimal(std::asin(d) * radToDeg) : BigDecimal(0);
    }
    if (op == "acos") {
        double d = static_cast<double>(value);
        return (d >= -1.0 && d <= 1.0) ? BigDecimal(std::acos(d) * radToDeg) : BigDecimal(0);
    }
    if (op == "atan") return BigDecimal(std::atan(static_cast<double>(value)) * radToDeg);
    return value;
}

} // namespace Rheno::Core
