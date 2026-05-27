#include "calculatorcore.h"

#include <cmath>
#include <limits>
#include <QVector>

namespace {

QString normalizeBig(QString s) {
    s = s.trimmed();
    if (s.isEmpty()) return "0";

    bool neg = false;
    if (s.startsWith('+')) s.remove(0, 1);
    if (s.startsWith('-')) {
        neg = true;
        s.remove(0, 1);
    }

    int i = 0;
    while (i < s.size() - 1 && s[i] == '0') ++i;
    s = s.mid(i);

    if (s == "0") neg = false;
    return neg ? ("-" + s) : s;
}

QString absBig(const QString& s) {
    return s.startsWith('-') ? s.mid(1) : s;
}

bool isNegBig(const QString& s) {
    return s.startsWith('-') && s != "0";
}

int cmpAbsBig(const QString& aIn, const QString& bIn) {
    const QString a = absBig(normalizeBig(aIn));
    const QString b = absBig(normalizeBig(bIn));
    if (a.size() != b.size()) return a.size() < b.size() ? -1 : 1;
    if (a == b) return 0;
    return a < b ? -1 : 1;
}

QString addAbsBig(const QString& aIn, const QString& bIn) {
    const QString a = absBig(normalizeBig(aIn));
    const QString b = absBig(normalizeBig(bIn));

    QString out;
    out.reserve((a.size() > b.size() ? a.size() : b.size()) + 1);

    int i = a.size() - 1;
    int j = b.size() - 1;
    int carry = 0;
    while (i >= 0 || j >= 0 || carry) {
        int da = (i >= 0) ? (a[i].unicode() - '0') : 0;
        int db = (j >= 0) ? (b[j].unicode() - '0') : 0;
        const int sum = da + db + carry;
        out.prepend(QChar('0' + (sum % 10)));
        carry = sum / 10;
        --i;
        --j;
    }
    return normalizeBig(out);
}

QString subAbsBig(const QString& aIn, const QString& bIn) {
    // assumes |a| >= |b|
    const QString a = absBig(normalizeBig(aIn));
    const QString b = absBig(normalizeBig(bIn));

    QString out;
    out.reserve(a.size());

    int i = a.size() - 1;
    int j = b.size() - 1;
    int borrow = 0;
    while (i >= 0) {
        int da = a[i].unicode() - '0' - borrow;
        int db = (j >= 0) ? (b[j].unicode() - '0') : 0;
        if (da < db) {
            da += 10;
            borrow = 1;
        } else {
            borrow = 0;
        }
        out.prepend(QChar('0' + (da - db)));
        --i;
        --j;
    }
    return normalizeBig(out);
}

QString addSignedBig(const QString& aIn, const QString& bIn) {
    const QString a = normalizeBig(aIn);
    const QString b = normalizeBig(bIn);
    const bool na = isNegBig(a);
    const bool nb = isNegBig(b);

    if (na == nb) {
        const QString sum = addAbsBig(a, b);
        if (na && sum != "0") return "-" + sum;
        return sum;
    }

    const int cmp = cmpAbsBig(a, b);
    if (cmp == 0) return "0";
    if (cmp > 0) {
        const QString d = subAbsBig(a, b);
        return na ? "-" + d : d;
    }
    const QString d = subAbsBig(b, a);
    return nb ? "-" + d : d;
}

QString mulAbsBig(const QString& aIn, const QString& bIn) {
    const QString a = absBig(normalizeBig(aIn));
    const QString b = absBig(normalizeBig(bIn));
    if (a == "0" || b == "0") return "0";

    QVector<int> acc(a.size() + b.size(), 0);
    for (int i = a.size() - 1; i >= 0; --i) {
        const int da = a[i].unicode() - '0';
        for (int j = b.size() - 1; j >= 0; --j) {
            const int db = b[j].unicode() - '0';
            const int idx = i + j + 1;
            const int prod = da * db + acc[idx];
            acc[idx] = prod % 10;
            acc[idx - 1] += prod / 10;
        }
    }

    QString out;
    bool started = false;
    for (int v : acc) {
        if (!started && v == 0) continue;
        started = true;
        out.append(QChar('0' + v));
    }
    return out.isEmpty() ? "0" : out;
}

QString mulSignedBig(const QString& a, const QString& b) {
    const bool neg = isNegBig(normalizeBig(a)) ^ isNegBig(normalizeBig(b));
    const QString p = mulAbsBig(a, b);
    if (neg && p != "0") return "-" + p;
    return p;
}

QString mulByDigitAbs(const QString& aIn, int digit) {
    if (digit <= 0) return "0";
    if (digit == 1) return absBig(normalizeBig(aIn));

    const QString a = absBig(normalizeBig(aIn));
    QString out;
    int carry = 0;
    for (int i = a.size() - 1; i >= 0; --i) {
        const int da = a[i].unicode() - '0';
        const int p = da * digit + carry;
        out.prepend(QChar('0' + (p % 10)));
        carry = p / 10;
    }
    while (carry) {
        out.prepend(QChar('0' + (carry % 10)));
        carry /= 10;
    }
    return normalizeBig(out);
}

QPair<QString, QString> divmodAbsBig(const QString& aIn, const QString& bIn) {
    const QString a = absBig(normalizeBig(aIn));
    const QString b = absBig(normalizeBig(bIn));
    if (b == "0") return {"0", "0"};
    if (cmpAbsBig(a, b) < 0) return {"0", a};

    QString quotient;
    QString rem = "0";
    quotient.reserve(a.size());

    for (int i = 0; i < a.size(); ++i) {
        rem = normalizeBig(rem + a.mid(i, 1));

        int qd = 0;
        for (int d = 9; d >= 1; --d) {
            const QString t = mulByDigitAbs(b, d);
            if (cmpAbsBig(t, rem) <= 0) {
                qd = d;
                rem = subAbsBig(rem, t);
                break;
            }
        }
        quotient.append(QChar('0' + qd));
    }

    return {normalizeBig(quotient), normalizeBig(rem)};
}

QPair<QString, QString> divmodSignedBig(const QString& aIn, const QString& bIn) {
    const QString a = normalizeBig(aIn);
    const QString b = normalizeBig(bIn);
    if (b == "0") return {"0", "0"};

    const bool negQ = isNegBig(a) ^ isNegBig(b);
    const bool negR = isNegBig(a);
    auto qr = divmodAbsBig(a, b);

    QString q = qr.first;
    QString r = qr.second;
    if (negQ && q != "0") q.prepend('-');
    if (negR && r != "0") r.prepend('-');
    return {normalizeBig(q), normalizeBig(r)};
}

QString applyBigBinary(const QString& aIn, const QString& bIn, const QString& op) {
    const QString a = normalizeBig(aIn);
    const QString b = normalizeBig(bIn);

    if (op == "+") return addSignedBig(a, b);
    if (op == "-") return addSignedBig(a, b.startsWith('-') ? b.mid(1) : ("-" + b));
    if (op == "*") return mulSignedBig(a, b);
    if (op == "/") return divmodSignedBig(a, b).first;
    if (op == "MOD") return divmodSignedBig(a, b).second;

    // Bitwise operators in big-decimal mode: fallback to long long if possible.
    bool okA = false;
    bool okB = false;
    const long long la = a.toLongLong(&okA);
    const long long lb = b.toLongLong(&okB);
    if (okA && okB) {
        if (op == "AND") return QString::number(la & lb);
        if (op == "OR") return QString::number(la | lb);
        if (op == "XOR") return QString::number(la ^ lb);
    }
    return a;
}

} // namespace

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
    if (m_bigMode && m_base != 10) {
        bool okCur = false;
        bool okAcc = false;
        m_current = normalizeBig(m_bigCurrent).toLongLong(&okCur);
        m_accumulator = normalizeBig(m_bigAccumulator).toLongLong(&okAcc);
        if (!okCur) m_current = 0;
        if (!okAcc) m_accumulator = 0;
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
        m_bigCurrent = QString::number(m_current);
        m_bigAccumulator = QString::number(m_accumulator);
    } else {
        bool okCur = false;
        bool okAcc = false;
        m_current = normalizeBig(m_bigCurrent).toLongLong(&okCur);
        m_accumulator = normalizeBig(m_bigAccumulator).toLongLong(&okAcc);
        if (!okCur) m_current = 0;
        if (!okAcc) m_accumulator = 0;
    }
    m_newInput = true;
}

QString CalculatorEngine::displayText() const {
    if (m_bigMode && m_base == 10)
        return normalizeBig(m_bigCurrent);

    if (m_floatMode) {
        if (!m_inputString.isEmpty())
            return m_inputString;
        return formatDouble(m_currentDouble);
    }
    return toBaseString(m_current, m_base, m_wordBits);
}

void CalculatorEngine::pressDigit(const QString& digit) {
    if (m_bigMode && m_base == 10) {
        if (digit == ".")
            return;

        QString cur = normalizeBig(m_bigCurrent);
        if (m_newInput || cur == "0") {
            cur = digit;
            m_newInput = false;
        } else {
            cur += digit;
        }
        m_bigCurrent = normalizeBig(cur);
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
    if (m_bigMode && m_base == 10) {
        if (!m_pendingOp.isEmpty())
            equals();

        m_bigAccumulator = m_bigCurrent;
        m_pendingOp = op;
        m_expression = normalizeBig(m_bigAccumulator) + " " + op;
        m_newInput = true;
        return;
    }

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

    if (m_bigMode && m_base == 10) {
        const QString a = normalizeBig(m_bigAccumulator);
        const QString b = normalizeBig(m_bigCurrent);
        const QString res = applyBigBinary(a, b, m_pendingOp);
        m_expression = a + " " + m_pendingOp + " " + b + " =";
        m_bigCurrent = normalizeBig(res);
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
    m_memory = 0;
    m_bigCurrent = "0";
    m_bigAccumulator = "0";
    m_bigMemory = "0";
}

void CalculatorEngine::clearEntry() {
    if (m_bigMode && m_base == 10)
        m_bigCurrent = "0";
    m_current = 0;
    m_newInput = true;
}

void CalculatorEngine::backspace() {
    if (m_bigMode && m_base == 10) {
        QString s = normalizeBig(m_bigCurrent);
        bool neg = s.startsWith('-');
        if (neg) s.remove(0, 1);
        if (s.length() > 1) s.chop(1); else s = "0";
        m_bigCurrent = normalizeBig((neg && s != "0") ? ("-" + s) : s);
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
        return;
    }

    QString s = toBaseString(m_current, m_base, m_wordBits);
    if (s.length() > 1) s.chop(1); else s = "0";
    m_current = fromBaseString(s, m_base);
}

void CalculatorEngine::negate() {
    if (m_bigMode && m_base == 10) {
        const QString s = normalizeBig(m_bigCurrent);
        m_bigCurrent = (s == "0") ? "0" : (s.startsWith('-') ? s.mid(1) : ("-" + s));
        return;
    }

    if (m_floatMode) {
        m_currentDouble = -m_currentDouble;
        m_inputString.clear();
    } else {
        m_current = maskToWidth(-m_current, m_wordBits);
    }
}

void CalculatorEngine::setPi() {
    if (m_bigMode && m_base == 10)
        return;

    m_floatMode = true;
    m_currentDouble = 3.14159265359;
    m_inputString.clear();
}

void CalculatorEngine::applyBitwiseOrFunction(const QString& op) {
    if (m_bigMode && m_base == 10) {
        const QString a = normalizeBig(m_bigAccumulator);
        const QString b = normalizeBig(m_bigCurrent);
        QString res = b;

        if (op == "AND" || op == "OR" || op == "XOR") {
            if (!m_pendingOp.isEmpty() && (m_pendingOp == "AND" || m_pendingOp == "OR" || m_pendingOp == "XOR")) {
                res = applyBigBinary(a, b, m_pendingOp);
                m_pendingOp.clear();
                m_newInput = true;
                m_bigCurrent = normalizeBig(res);
                return;
            }
            m_bigAccumulator = b;
            m_pendingOp = op;
            m_expression = b + " " + op;
            m_newInput = true;
            return;
        }

        if (op == "NOT") {
            bool ok = false;
            const long long v = b.toLongLong(&ok);
            if (!ok) return;
            res = QString::number(~v);
        }
        else if (op == "LSL") res = mulSignedBig(b, "2");
        else if (op == "LSR") res = divmodSignedBig(b, "2").first;
        else if (op == "MS") {
            m_bigMemory = b;
            m_expression = "M← " + b;
            return;
        } else if (op == "MR") {
            res = m_bigMemory;
            m_expression = "M→ " + normalizeBig(m_bigMemory);
        } else if (op == "MC") {
            m_bigMemory = "0";
            m_expression = "M cleared";
            return;
        } else if (op == "abs") {
            res = isNegBig(b) ? b.mid(1) : b;
        } else {
            // Unsupported in big-int mode (e.g. sqrt, 1/x, rol/ror)
            return;
        }

        m_bigCurrent = normalizeBig(res);
        m_newInput = true;
        return;
    }

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

