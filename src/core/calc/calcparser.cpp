#include "calcparser.h"

#include <limits>
#include <QVector>

namespace Rheno::Core {

QString bigToDisplayString(const BigDecimal& v) {
    std::string s = v.str(12);
    if (s.empty() || s == "-0") s = "0";
    return QString::fromStdString(s);
}

QString bigToTokenString(const BigDecimal& v) {
    constexpr auto digits = std::numeric_limits<BigDecimal>::max_digits10;
    std::string s = v.str(digits);
    if (s.empty() || s == "-0") s = "0";
    return QString::fromStdString(s);
}

BigDecimal qStringToBig(const QString& s) {
    try {
        return BigDecimal(s.toStdString());
    } catch (...) {
        return BigDecimal(0);
    }
}

bool isBinaryOperatorToken(const QString& token) {
    return token == "+" || token == "-" || token == "*" || token == "/" ||
           token == "MOD" || token == "AND" || token == "OR" || token == "XOR";
}

int precedenceOf(const QString& token) {
    if (token == "OR") return 1;
    if (token == "XOR") return 2;
    if (token == "AND") return 3;
    if (token == "+" || token == "-") return 4;
    if (token == "*" || token == "/" || token == "MOD") return 5;
    return 0;
}

bool toRpn(const QStringList& tokens, QStringList* outRpn) {
    outRpn->clear();
    QVector<QString> opStack;

    for (const QString& tk : tokens) {
        if (tk == "(") {
            opStack.push_back(tk);
            continue;
        }
        if (tk == ")") {
            bool foundLeft = false;
            while (!opStack.isEmpty()) {
                const QString top = opStack.back();
                opStack.pop_back();
                if (top == "(") {
                    foundLeft = true;
                    break;
                }
                outRpn->append(top);
            }
            if (!foundLeft)
                return false;
            continue;
        }
        if (isBinaryOperatorToken(tk)) {
            while (!opStack.isEmpty() && isBinaryOperatorToken(opStack.back()) &&
                   precedenceOf(opStack.back()) >= precedenceOf(tk)) {
                outRpn->append(opStack.back());
                opStack.pop_back();
            }
            opStack.push_back(tk);
            continue;
        }

        outRpn->append(tk);
    }

    while (!opStack.isEmpty()) {
        if (opStack.back() == "(" || opStack.back() == ")")
            return false;
        outRpn->append(opStack.back());
        opStack.pop_back();
    }
    return true;
}

bool evalIntRpn(const QStringList& rpn, int base, int bits, long long* result) {
    QVector<long long> st;
    for (const QString& tk : rpn) {
        if (isBinaryOperatorToken(tk)) {
            if (st.size() < 2)
                return false;
            const long long b = st.back(); st.pop_back();
            const long long a = st.back(); st.pop_back();
            st.push_back(maskToWidth(applyBinary(a, b, tk), bits));
            continue;
        }

        bool ok = false;
        const long long v = tk.toLongLong(&ok, base);
        if (!ok)
            return false;
        st.push_back(maskToWidth(v, bits));
    }

    if (st.size() != 1)
        return false;
    *result = maskToWidth(st.back(), bits);
    return true;
}

bool evalDoubleRpn(const QStringList& rpn, double* result) {
    QVector<double> st;
    for (const QString& tk : rpn) {
        if (isBinaryOperatorToken(tk)) {
            if (st.size() < 2)
                return false;
            const double b = st.back(); st.pop_back();
            const double a = st.back(); st.pop_back();
            st.push_back(applyBinary(a, b, tk));
            continue;
        }

        bool ok = false;
        const double v = tk.toDouble(&ok);
        if (!ok)
            return false;
        st.push_back(v);
    }

    if (st.size() != 1)
        return false;
    *result = st.back();
    return true;
}

bool evalBigRpn(const QStringList& rpn, BigDecimal* result) {
    QVector<BigDecimal> st;
    for (const QString& tk : rpn) {
        if (isBinaryOperatorToken(tk)) {
            if (st.size() < 2)
                return false;
            const auto b = st.back(); st.pop_back();
            const auto a = st.back(); st.pop_back();
            st.push_back(applyBigBinary(a, b, tk));
            continue;
        }
        st.push_back(qStringToBig(tk));
    }
    if (st.size() != 1)
        return false;
    *result = st.back();
    return true;
}

} // namespace Rheno::Core
