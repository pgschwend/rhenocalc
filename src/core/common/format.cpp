#include "format.h"

#include <QLocale>
#include <QRegularExpression>

#include <cmath>

namespace Rheno::Core {

QString formatEngineeringValue(double value, const QString& unit) {
    if (!std::isfinite(value) || value == 0.0)
        return "—";

    static const char* prefixes[] = {"p", "n", "u", "m", "", "k", "M", "G", "T"};
    constexpr int baseIndex = 4;

    double absVal = std::abs(value);
    int exp = 0;

    if (absVal >= 1.0) {
        while (absVal >= 1000.0 && exp < 4) {
            absVal /= 1000.0;
            ++exp;
        }
    } else {
        while (absVal < 1.0 && exp > -4) {
            absVal *= 1000.0;
            --exp;
        }
    }

    if (value < 0)
        absVal = -absVal;

    const int prefixIdx = baseIndex + exp;
    if (prefixIdx < 0 || prefixIdx > 8)
        return QString::number(value, 'g', 4) + " " + unit;

    return QString::number(absVal, 'f', 3).remove(QRegularExpression("\\.?0+$")) + " " + prefixes[prefixIdx] + unit;
}

QString formatMoneyLocalized(double value, int decimals) {
    return QLocale::system().toString(value, 'f', decimals);
}

} // namespace Rheno::Core


