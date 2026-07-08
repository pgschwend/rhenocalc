#include "calculator.h"

#include <cmath>
#include <limits>

#include <boost/math/constants/constants.hpp>

namespace Rheno::Core {

void CalculatorEngine::setBase(int base) {
    m_base = base;
    if (m_bigMode && m_base != 10) {
        m_current = static_cast<long long>(m_bigCurrent);
        m_accumulator = static_cast<long long>(m_bigAccumulator);
    }
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

void CalculatorEngine::setBigMode(bool enabled) {
    if (m_bigMode == enabled)
        return;

    m_bigMode = enabled;
    if (m_bigMode) {
        m_floatMode = false;
        m_inputString.clear();
        m_bigCurrent = BigDecimal(m_current);
        m_bigAccumulator = BigDecimal(m_accumulator);
    } else {
        m_current = static_cast<long long>(m_bigCurrent);
        m_accumulator = static_cast<long long>(m_bigAccumulator);
    }
    m_newInput = true;
}

bool CalculatorEngine::isClearState() const {
    if (!m_pendingOp.isEmpty() || !m_expression.isEmpty())
        return false;

    if (m_bigMode && m_base == 10)
        return m_bigCurrent == 0 && m_newInput;

    if (m_floatMode) {
        if (!m_inputString.isEmpty())
            return false;
        return std::fabs(m_currentDouble) <= std::numeric_limits<double>::epsilon() && m_newInput;
    }

    return m_current == 0 && m_newInput;
}

QString CalculatorEngine::displayText() const {
    if (m_bigMode && m_base == 10) {
        if (!m_inputString.isEmpty())
            return m_inputString;
        return bigToDisplayString(m_bigCurrent);
    }

    if (m_floatMode) {
        if (!m_inputString.isEmpty())
            return m_inputString;
        return formatDouble(m_currentDouble);
    }
    return toBaseString(m_current, m_base, m_wordBits);
}

QString CalculatorEngine::currentOperandToken() const {
    if (m_bigMode && m_base == 10) {
        if (!m_inputString.isEmpty())
            return m_inputString;
        return bigToTokenString(m_bigCurrent);
    }
    if (m_floatMode) {
        if (!m_inputString.isEmpty())
            return m_inputString;
        return formatDouble(m_currentDouble);
    }
    return toBaseString(m_current, m_base, m_wordBits);
}

QString CalculatorEngine::formatTokenForExpression(const QString& token) const {
    if (!(m_bigMode && m_base == 10))
        return token;

    if (token == "(" || token == ")" || isBinaryOperatorToken(token))
        return token;

    // Keep active user input exactly as typed (e.g. trailing dot).
    if (!m_inputString.isEmpty() && token == m_inputString)
        return token;

    return bigToDisplayString(qStringToBig(token));
}

QString CalculatorEngine::expressionFromTokens(const QStringList& tokens) const {
    QStringList displayTokens;
    displayTokens.reserve(tokens.size());
    for (const QString& token : tokens)
        displayTokens.append(formatTokenForExpression(token));
    return displayTokens.join(" ");
}

void CalculatorEngine::syncExpressionOperand() {
    if (m_infixTokens.isEmpty())
        return;

    const QString value = currentOperandToken();
    const QString last = m_infixTokens.last();
    if (isBinaryOperatorToken(last) || last == "(") {
        m_infixTokens.append(value);
    } else if (last != ")") {
        m_infixTokens.last() = value;
    }
    m_expression = expressionFromTokens(m_infixTokens);
}

void CalculatorEngine::resetExpressionBuilder() {
    m_infixTokens.clear();
    m_openParens = 0;
}

void CalculatorEngine::pressDigit(const QString& digit) {
    if (digit == "(") {
        pressLeftParen();
        return;
    }
    if (digit == ")") {
        pressRightParen();
        return;
    }

    if (m_bigMode && m_base == 10) {
        if (digit == ".") {
            QString cur = !m_inputString.isEmpty() ? m_inputString : bigToTokenString(m_bigCurrent);
            if (cur.contains('.')) return;
            if (m_newInput) {
                m_inputString = "0.";
                m_bigCurrent = BigDecimal(0);
                m_newInput = false;
            } else {
                m_inputString = cur + ".";
            }
            syncExpressionOperand();
            return;
        }

        if (m_newInput) {
            m_inputString = digit;
            m_bigCurrent = qStringToBig(digit);
            m_newInput = false;
        } else {
            if (m_inputString.isEmpty())
                m_inputString = bigToTokenString(m_bigCurrent);
            if (m_inputString == "0")
                m_inputString = digit;
            else
                m_inputString += digit;
            m_bigCurrent = qStringToBig(m_inputString);
        }
        syncExpressionOperand();
        return;
    }

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
        syncExpressionOperand();
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
        syncExpressionOperand();
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
    syncExpressionOperand();
}

void CalculatorEngine::pressLeftParen() {
    if (!m_infixTokens.isEmpty()) {
        const QString last = m_infixTokens.last();
        if (last != "(" && !isBinaryOperatorToken(last))
            m_infixTokens.append("*");
    }

    m_infixTokens.append("(");
    ++m_openParens;
    m_expression = expressionFromTokens(m_infixTokens);
    m_newInput = true;
}

void CalculatorEngine::pressRightParen() {
    if (m_openParens <= 0)
        return;
    if (!m_infixTokens.isEmpty()) {
        const QString last = m_infixTokens.last();
        if (isBinaryOperatorToken(last) || last == "(") {
            if (!m_newInput)
                syncExpressionOperand();
        }
    }

    if (m_infixTokens.isEmpty())
        return;

    const QString last = m_infixTokens.last();
    if (last == "(" || isBinaryOperatorToken(last))
        return;

    m_infixTokens.append(")");
    --m_openParens;
    m_expression = expressionFromTokens(m_infixTokens);
    m_newInput = true;
}

void CalculatorEngine::pressOperator(const QString& op) {
    if (!isBinaryOperatorToken(op))
        return;

    if (m_infixTokens.isEmpty()) {
        m_infixTokens << currentOperandToken() << op;
    } else {
        const QString last = m_infixTokens.last();
        if (isBinaryOperatorToken(last)) {
            m_infixTokens.last() = op;
        } else if (last != "(") {
            m_infixTokens << op;
        }
    }

    m_expression = expressionFromTokens(m_infixTokens);
    m_pendingOp.clear();
    m_newInput = true;
    m_inputString.clear();
}

void CalculatorEngine::equals() {
    if (!m_infixTokens.isEmpty()) {
        QStringList tokens = m_infixTokens;
        if (isBinaryOperatorToken(tokens.last())) {
            if (!m_newInput) {
                tokens << currentOperandToken();
            } else if (tokens.size() >= 2 && !isBinaryOperatorToken(tokens[tokens.size() - 2]) && tokens[tokens.size() - 2] != "(") {
                tokens << tokens[tokens.size() - 2];
            } else {
                m_expression = "Error";
                resetExpressionBuilder();
                return;
            }
        }

        for (int i = 0; i < m_openParens; ++i)
            tokens << ")";

        QStringList rpn;
        if (!toRpn(tokens, &rpn)) {
            m_expression = "Error";
            resetExpressionBuilder();
            return;
        }

        if (m_bigMode && m_base == 10) {
            BigDecimal res;
            if (!evalBigRpn(rpn, &res)) {
                m_expression = "Error";
                resetExpressionBuilder();
                return;
            }
            m_bigCurrent = res;
            m_inputString.clear();
        } else if (m_floatMode) {
            double res = 0.0;
            if (!evalDoubleRpn(rpn, &res)) {
                m_expression = "Error";
                resetExpressionBuilder();
                return;
            }
            m_currentDouble = res;
            m_inputString.clear();
        } else {
            long long res = 0;
            if (!evalIntRpn(rpn, m_base, m_wordBits, &res)) {
                m_expression = "Error";
                resetExpressionBuilder();
                return;
            }
            m_current = maskToWidth(res, m_wordBits);
        }

        m_expression = expressionFromTokens(tokens) + " =";
        resetExpressionBuilder();
        m_pendingOp.clear();
        m_newInput = true;
        return;
    }

    if (m_pendingOp.isEmpty()) return;

    // Handle special binary operations (POW, NROOT, LOGXY)
    if (m_pendingOp == "POW" || m_pendingOp == "NROOT" || m_pendingOp == "LOGXY") {
        double aVal, bVal;
        if (m_bigMode && m_base == 10) {
            aVal = static_cast<double>(m_bigAccumulator);
            bVal = static_cast<double>(m_bigCurrent);
        } else if (m_floatMode) {
            aVal = m_accumulatorDouble;
            bVal = m_currentDouble;
        } else {
            aVal = static_cast<double>(m_accumulator);
            bVal = static_cast<double>(m_current);
        }

        double result = 0.0;
        QString opSymbol;
        if (m_pendingOp == "POW") {
            result = std::pow(aVal, bVal);
            opSymbol = "^";
        } else if (m_pendingOp == "NROOT") {
            result = bVal != 0.0 ? std::pow(aVal, 1.0 / bVal) : 0.0;
            opSymbol = "ʸ√";
        } else if (m_pendingOp == "LOGXY") {
            result = (bVal > 0 && bVal != 1.0 && aVal > 0) ? std::log(aVal) / std::log(bVal) : 0.0;
            opSymbol = "log";
        }

        if (m_bigMode && m_base == 10) {
            m_expression = bigToDisplayString(m_bigAccumulator) + " " + opSymbol + " " + bigToDisplayString(m_bigCurrent) + " =";
            m_bigCurrent = BigDecimal(result);
        } else {
            m_expression = formatDouble(aVal) + " " + opSymbol + " " + formatDouble(bVal) + " =";
            m_currentDouble = result;
            m_floatMode = true;
        }
        m_inputString.clear();
        m_pendingOp.clear();
        m_newInput = true;
        return;
    }

    if (m_bigMode && m_base == 10) {
        const QString a = bigToDisplayString(m_bigAccumulator);
        const QString b = bigToDisplayString(m_bigCurrent);
        m_bigCurrent = applyBigBinary(m_bigAccumulator, m_bigCurrent, m_pendingOp);
        m_expression = a + " " + m_pendingOp + " " + b + " =";
        m_inputString.clear();
        m_pendingOp.clear();
        m_newInput = true;
        return;
    }

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
    m_bigCurrent = BigDecimal(0);
    m_bigAccumulator = BigDecimal(0);
    resetExpressionBuilder();
}

void CalculatorEngine::clearAllAndMemory() {
    clearAll();
    m_memory = 0;
    m_bigMemory = BigDecimal(0);
}

void CalculatorEngine::clearEntry() {
    if (m_bigMode && m_base == 10) {
        m_bigCurrent = BigDecimal(0);
        m_inputString.clear();
    }
    m_current = 0;
    m_newInput = true;
    syncExpressionOperand();
}

void CalculatorEngine::backspace() {
    if (m_bigMode && m_base == 10) {
        QString s = !m_inputString.isEmpty() ? m_inputString : bigToTokenString(m_bigCurrent);
        bool neg = s.startsWith('-');
        if (neg) s.remove(0, 1);
        if (s.length() > 1) s.chop(1); else s = "0";
        if (neg && s != "0") s.prepend('-');
        m_inputString = s.contains('.') ? s : QString();
        m_bigCurrent = qStringToBig(s);
        syncExpressionOperand();
        return;
    }

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
        syncExpressionOperand();
        return;
    }

    QString s = toBaseString(m_current, m_base, m_wordBits);
    if (s.length() > 1) s.chop(1); else s = "0";
    m_current = fromBaseString(s, m_base);
    syncExpressionOperand();
}

void CalculatorEngine::negate() {
    if (m_bigMode && m_base == 10) {
        m_bigCurrent = -m_bigCurrent;
        m_inputString.clear();
        syncExpressionOperand();
        return;
    }

    if (m_floatMode) {
        m_currentDouble = -m_currentDouble;
        m_inputString.clear();
    } else {
        m_current = maskToWidth(-m_current, m_wordBits);
    }
    syncExpressionOperand();
}

void CalculatorEngine::setPi() {
    if (m_bigMode && m_base == 10) {
        m_bigCurrent = boost::math::constants::pi<BigDecimal>();
        m_inputString.clear();
        syncExpressionOperand();
        return;
    }

    m_floatMode = true;
    m_currentDouble = boost::math::constants::pi<double>();
    m_inputString.clear();
    syncExpressionOperand();
}

void CalculatorEngine::setEuler() {
    if (m_bigMode && m_base == 10) {
        m_bigCurrent = boost::math::constants::e<BigDecimal>();
        m_inputString.clear();
        syncExpressionOperand();
        return;
    }

    m_floatMode = true;
    m_currentDouble = boost::math::constants::e<double>();
    m_inputString.clear();
    syncExpressionOperand();
}

void CalculatorEngine::applyBitwiseOrFunction(const QString& op) {
    if (m_bigMode && m_base == 10) {
        const QString bStr = bigToDisplayString(m_bigCurrent);

        if (op == "AND" || op == "OR" || op == "XOR" || op == "POW" || op == "NROOT" || op == "LOGXY") {
            if (!m_pendingOp.isEmpty() && (m_pendingOp == "AND" || m_pendingOp == "OR" || m_pendingOp == "XOR" ||
                                            m_pendingOp == "POW" || m_pendingOp == "NROOT" || m_pendingOp == "LOGXY")) {
                if (m_pendingOp == "POW") {
                    double base = static_cast<double>(m_bigAccumulator);
                    double exp = static_cast<double>(m_bigCurrent);
                    m_bigCurrent = BigDecimal(std::pow(base, exp));
                } else if (m_pendingOp == "NROOT") {
                    double x = static_cast<double>(m_bigAccumulator);
                    double y = static_cast<double>(m_bigCurrent);
                    m_bigCurrent = y != 0.0 ? BigDecimal(std::pow(x, 1.0 / y)) : BigDecimal(0);
                } else if (m_pendingOp == "LOGXY") {
                    double x = static_cast<double>(m_bigAccumulator);
                    double y = static_cast<double>(m_bigCurrent);
                    m_bigCurrent = (y > 0 && y != 1.0 && x > 0) ? BigDecimal(std::log(x) / std::log(y)) : BigDecimal(0);
                } else {
                    m_bigCurrent = applyBigBinary(m_bigAccumulator, m_bigCurrent, m_pendingOp);
                }
                m_inputString.clear();
                m_pendingOp.clear();
                m_newInput = true;
                return;
            }
            m_bigAccumulator = m_bigCurrent;
            m_pendingOp = op;
            QString opSymbol = op;
            if (op == "POW") opSymbol = "^";
            if (op == "NROOT") opSymbol = "ʸ√";
            if (op == "LOGXY") opSymbol = "log";
            m_expression = bStr + " " + opSymbol;
            m_newInput = true;
            return;
        }

        if (op == "SQ" || op == "SQRT" || op == "1/x" || op == "log" || op == "ln" ||
            op == "NOT" || op == "LSL" || op == "LSR" ||
            op == "sin" || op == "cos" || op == "tan" ||
            op == "asin" || op == "acos" || op == "atan") {
            m_bigCurrent = applyBigUnary(m_bigCurrent, op);
            m_inputString.clear();
            m_newInput = true;
            return;
        }

        if (op == "MS") {
            m_bigMemory = m_bigCurrent;
            m_expression = "M← " + bStr;
            return;
        }
        if (op == "MR") {
            m_bigCurrent = m_bigMemory;
            m_inputString.clear();
            m_expression = "M→ " + bigToDisplayString(m_bigMemory);
            m_newInput = true;
            return;
        }
        if (op == "MC") {
            m_bigMemory = BigDecimal(0);
            m_expression = "M cleared";
            return;
        }

        // Unsupported (ROL, ROR)
        return;
    }

    const long long a = m_accumulator;
    const long long b = m_current;
    long long res = m_current;

    if (op == "AND" || op == "OR" || op == "XOR" || op == "POW" || op == "NROOT" || op == "LOGXY") {
        if (!m_pendingOp.isEmpty() && (m_pendingOp == "AND" || m_pendingOp == "OR" || m_pendingOp == "XOR" ||
                                        m_pendingOp == "POW" || m_pendingOp == "NROOT" || m_pendingOp == "LOGXY")) {
            if (m_floatMode || m_pendingOp == "POW" || m_pendingOp == "NROOT" || m_pendingOp == "LOGXY") {
                double aVal = (m_pendingOp == "POW" || m_pendingOp == "NROOT" || m_pendingOp == "LOGXY")
                    ? (m_floatMode ? m_accumulatorDouble : static_cast<double>(a))
                    : static_cast<double>(a);
                double bVal = m_floatMode ? m_currentDouble : static_cast<double>(b);
                if (m_pendingOp == "POW") {
                    m_currentDouble = std::pow(aVal, bVal);
                } else if (m_pendingOp == "NROOT") {
                    m_currentDouble = bVal != 0.0 ? std::pow(aVal, 1.0 / bVal) : 0.0;
                } else if (m_pendingOp == "LOGXY") {
                    m_currentDouble = (bVal > 0 && bVal != 1.0 && aVal > 0) ? std::log(aVal) / std::log(bVal) : 0.0;
                }
                m_floatMode = true;
                m_inputString.clear();
            } else {
                res = applyBinary(a, b, m_pendingOp);
                m_current = maskToWidth(res, m_wordBits);
            }
            m_pendingOp.clear();
            m_newInput = true;
            return;
        }
        m_accumulator = b;
        m_accumulatorDouble = m_floatMode ? m_currentDouble : static_cast<double>(b);
        m_pendingOp = op;
        QString opSymbol = op;
        if (op == "POW") opSymbol = "^";
        if (op == "NROOT") opSymbol = "ʸ√";
        if (op == "LOGXY") opSymbol = "log";
        m_expression = (m_floatMode ? formatDouble(m_currentDouble) : toBaseString(b, m_base, m_wordBits)) + " " + opSymbol;
        m_newInput = true;
        return;
    }

    if (op == "SQ" || op == "SQRT") {
        if (m_floatMode) {
            m_currentDouble = applyUnaryDouble(m_currentDouble, op);
            m_inputString.clear();
            return;
        }
        res = applyUnaryInt(b, op, m_wordBits);
    } else if (op == "log" || op == "ln" ||
               op == "sin" || op == "cos" || op == "tan" ||
               op == "asin" || op == "acos" || op == "atan" ||
               op == "sinh" || op == "cosh" || op == "tanh") {
        const double in = m_floatMode ? m_currentDouble : static_cast<double>(b);
        m_currentDouble = applyUnaryDouble(in, op);
        m_floatMode = true;
        m_inputString.clear();
        m_expression = op + "(" + formatDouble(in) + ") =";
        m_newInput = true;
        return;
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

} // namespace Rheno::Core
