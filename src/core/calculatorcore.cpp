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

void CalculatorEngine::setBase(int base) {
    m_base = base;
    if (m_base != 10 && m_floatMode) {
        m_floatMode = false;
        m_inputString.clear();
        m_current = static_cast<long long>(m_currentDouble);
        m_currentDouble = 0.0;
    }
    m_newInput = true;
}

void CalculatorEngine::setWordBits(int bits) {
    m_wordBits = bits;
    m_current = maskToWidth(m_current, m_wordBits);
}

QString CalculatorEngine::displayText() const {
    if (m_floatMode) {
        if (!m_inputString.isEmpty())
            return m_inputString;
        return formatDouble(m_currentDouble);
    }
    return toBaseString(m_current, m_base, m_wordBits);
}

void CalculatorEngine::pressDigit(const QString& digit) {
    if (digit == ".") {
        if (m_base != 10) return;
        if (m_floatMode && m_inputString.contains('.')) return;
        m_floatMode = true;
        if (m_newInput) {
            m_inputString = "0.";
            m_currentDouble = 0.0;
            m_newInput = false;
        } else if (m_inputString.isEmpty()) {
            m_inputString = QString::number(m_current) + ".";
            m_currentDouble = static_cast<double>(m_current);
        } else {
            m_inputString += ".";
            m_currentDouble = m_inputString.toDouble();
        }
        return;
    }

    if (m_floatMode) {
        if (m_newInput) {
            m_inputString = digit;
            m_currentDouble = digit.toDouble();
            m_newInput = false;
        } else {
            m_inputString += digit;
            m_currentDouble = m_inputString.toDouble();
        }
        return;
    }

    if (m_newInput) {
        m_current = 0;
        m_newInput = false;
    }

    QString cur = toBaseString(m_current, m_base, m_wordBits);
    if (cur == "0") cur = digit;
    else cur += digit;
    m_current = maskToWidth(fromBaseString(cur, m_base), m_wordBits);
}

void CalculatorEngine::pressOperator(const QString& op) {
    if (!m_pendingOp.isEmpty())
        equals();

    if (m_base == 10 && !m_floatMode) {
        m_floatMode = true;
        m_currentDouble = static_cast<double>(m_current);
        m_inputString.clear();
    }

    if (m_floatMode) {
        m_accumulatorDouble = m_currentDouble;
        m_inputString.clear();
        m_pendingOp = op;
        m_expression = formatDouble(m_accumulatorDouble) + " " + op;
    } else {
        m_accumulator = m_current;
        m_pendingOp = op;
        m_expression = toBaseString(m_accumulator, m_base, m_wordBits) + " " + op;
    }
    m_newInput = true;
}

void CalculatorEngine::equals() {
    if (m_pendingOp.isEmpty()) return;

    if (m_floatMode) {
        const double a = m_accumulatorDouble;
        const double b = m_currentDouble;
        const double res = applyBinary(a, b, m_pendingOp);
        m_expression = formatDouble(a) + " " + m_pendingOp + " " + formatDouble(b) + " =";
        m_currentDouble = res;
        m_inputString.clear();
        m_pendingOp.clear();
        m_newInput = true;
        return;
    }

    const long long a = m_accumulator;
    const long long b = m_current;
    const long long res = applyBinary(a, b, m_pendingOp);
    m_expression = toBaseString(a, m_base, m_wordBits) + " " + m_pendingOp + " " + toBaseString(b, m_base, m_wordBits) + " =";
    m_current = maskToWidth(res, m_wordBits);
    m_pendingOp.clear();
    m_newInput = true;
}

void CalculatorEngine::clearAll() {
    m_current = 0;
    m_accumulator = 0;
    m_pendingOp.clear();
    m_currentDouble = 0.0;
    m_accumulatorDouble = 0.0;
    m_floatMode = false;
    m_inputString.clear();
    m_newInput = true;
    m_expression.clear();
}

void CalculatorEngine::clearEntry() {
    m_current = 0;
    m_newInput = true;
}

void CalculatorEngine::backspace() {
    if (m_floatMode) {
        if (m_inputString.isEmpty()) return;
        m_inputString.chop(1);
        if (m_inputString.isEmpty() || m_inputString == "-" || !m_inputString.contains('.')) {
            m_floatMode = false;
            m_current = m_inputString.isEmpty() ? 0 : static_cast<long long>(m_inputString.toDouble());
            m_inputString.clear();
        } else {
            m_currentDouble = m_inputString.toDouble();
        }
        return;
    }

    QString s = toBaseString(m_current, m_base, m_wordBits);
    if (s.length() > 1) s.chop(1); else s = "0";
    m_current = fromBaseString(s, m_base);
}

void CalculatorEngine::negate() {
    if (m_floatMode) {
        m_currentDouble = -m_currentDouble;
        m_inputString.clear();
    } else {
        m_current = maskToWidth(-m_current, m_wordBits);
    }
}

void CalculatorEngine::setPi() {
    m_floatMode = true;
    m_currentDouble = 3.14159265359;
    m_inputString.clear();
}

void CalculatorEngine::applyBitwiseOrFunction(const QString& op) {
    const long long a = m_accumulator;
    const long long b = m_current;
    long long res = m_current;

    if (op == "AND" || op == "OR" || op == "XOR") {
        if (!m_pendingOp.isEmpty() && (m_pendingOp == "AND" || m_pendingOp == "OR" || m_pendingOp == "XOR")) {
            res = applyBinary(a, b, m_pendingOp);
            m_pendingOp.clear();
            m_newInput = true;
            m_current = maskToWidth(res, m_wordBits);
            return;
        }
        m_accumulator = b;
        m_pendingOp = op;
        m_expression = toBaseString(b, m_base, m_wordBits) + " " + op;
        m_newInput = true;
        return;
    }

    if (op == "SQ" || op == "SQRT" || op == "abs") {
        if (m_floatMode) {
            m_currentDouble = applyUnaryDouble(m_currentDouble, op);
            m_inputString.clear();
            return;
        }
        res = applyUnaryInt(b, op, m_wordBits);
    } else if (op == "1/x") {
        if (m_floatMode) {
            m_currentDouble = applyUnaryDouble(m_currentDouble, op);
            m_expression = "1 / " + formatDouble(m_currentDouble) + " =";
            m_inputString.clear();
            return;
        }
        m_expression = "1 / " + toBaseString(b, m_base, m_wordBits) + " = ";
        if (b != 0) {
            m_floatMode = true;
            m_currentDouble = 1.0 / static_cast<double>(b);
            m_inputString.clear();
            m_newInput = true;
        }
        return;
    } else if (op == "NOT" || op == "LSL" || op == "LSR" || op == "ROL" || op == "ROR") {
        res = applyUnaryInt(b, op, m_wordBits);
    } else if (op == "MS") {
        m_memory = b;
        m_expression = "M← " + toBaseString(b, m_base, m_wordBits);
        return;
    } else if (op == "MR") {
        res = m_memory;
        if (m_floatMode) {
            m_currentDouble = static_cast<double>(m_memory);
            m_inputString.clear();
        }
        m_expression = "M→ " + toBaseString(m_memory, m_base, m_wordBits);
    } else if (op == "MC") {
        m_memory = 0;
        m_expression = "M cleared";
        return;
    }

    m_current = res;
    m_newInput = true;
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

