#include "calculatorcore.h"

#include <cmath>
#include <limits>

namespace CalculatorCore {

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

double applyBinary(double a, double b, const QString& op) {
    if (op == "+") return a + b;
    if (op == "-") return a - b;
    if (op == "*") return a * b;
    if (op == "/") return b != 0.0 ? a / b : std::numeric_limits<double>::infinity();
    if (op == "MOD") return std::fmod(a, b);
    return a;
}

long long applyUnaryInt(long long value, const QString& op, int bits) {
    if (op == "SQ") return maskToWidth(value * value, bits);
    if (op == "SQRT") return value >= 0 ? static_cast<long long>(std::sqrt(static_cast<double>(value))) : 0;
    if (op == "abs") return std::abs(value);
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

double applyUnaryDouble(double value, const QString& op) {
    if (op == "SQ") return value * value;
    if (op == "SQRT") return value >= 0.0 ? std::sqrt(value) : std::numeric_limits<double>::quiet_NaN();
    if (op == "abs") return std::fabs(value);
    if (op == "1/x") return value != 0.0 ? 1.0 / value : std::numeric_limits<double>::infinity();
    return value;
}

} // namespace CalculatorCore

