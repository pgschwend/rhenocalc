#include "unitconverter.h"

#include <cmath>
#include <limits>

namespace Rheno::Core {

QList<QList<UnitDef>> defaultCategories() {
    QList<QList<UnitDef>> categories;

    categories.append({
        {"Hz", 1.0}, {"kHz", 1e3}, {"MHz", 1e6}, {"GHz", 1e9}, {"THz", 1e12}, {"mHz", 1e-3},
        {"s", 0.0}, {"ms", 0.0}, {"us", 0.0},
    });

    categories.append({
        {"s", 1.0}, {"ms", 1e-3}, {"µs", 1e-6}, {"ns", 1e-9}, {"ps", 1e-12},
        {"min", 60.0}, {"h", 3600.0}, {"day", 86400.0},
    });

    categories.append({{"bar", 100000.0}, {"kPa", 1000.0}, {"psi", 6894.757293168}});
    categories.append({{"km", 1000.0}, {"mile", 1609.344}, {"naut mile", 1852.0}});
    categories.append({{"m/s", 1.0}, {"km/h", 0.2777777777777778}, {"knots", 0.5144444444444445}, {"mph", 0.44704}, {"nmph", 0.5144444444444445}});

    categories.append({
        {"bps", 1.0}, {"kbps", 1e3}, {"Mbps", 1e6}, {"Gbps", 1e9},
        {"Byte/s", 8.0}, {"KB/s", 8e3}, {"MB/s", 8e6}, {"GB/s", 8e9},
    });

    categories.append({
        {"Byte", 1.0}, {"KB", 1024.0}, {"MB", 1024.0 * 1024.0}, {"GB", 1024.0 * 1024.0 * 1024.0},
        {"TB", 1024.0 * 1024.0 * 1024.0 * 1024.0}, {"bit", 0.125}, {"kbit", 125.0}, {"Mbit", 125000.0},
    });

    categories.append({{"°C", 1.0}, {"°F", 1.0}, {"K", 1.0}});

    categories.append({
        {"W", 1.0}, {"mW", 1e-3}, {"µW", 1e-6}, {"kW", 1e3}, {"MW", 1e6}, {"dBm", 1.0}, {"hp", 745.7},
    });

    categories.append({
        {"J", 1.0}, {"mJ", 1e-3}, {"µJ", 1e-6}, {"kJ", 1e3}, {"MJ", 1e6},
        {"Wh", 3600.0}, {"kWh", 3.6e6}, {"eV", 1.60218e-19},
    });

    categories.append({{"°", 1.0}, {"rad", 180.0 / M_PI}, {"mrad", 0.18 / M_PI}, {"grad", 0.9}});

    return categories;
}

QStringList defaultCategoryNames() {
    return {
        "Frequency (Hz)", "Time / Period (s)", "Pressure", "Distance", "Speed", "Data Rate (bps)",
        "Data Size (bytes)", "Temperature", "Power (W)", "Energy (J)", "Angle"
    };
}

static QString formatResult(double value) {
    if (std::isinf(value))
        return "inf";
    if (std::abs(value) >= 1e9 || (std::abs(value) < 1e-6 && value != 0.0))
        return QString::number(value, 'e', 6);
    return QString::number(value, 'g', 10);
}

ConversionResult convert(
    double fromValue,
    int categoryIndex,
    int fromUnitIndex,
    int toUnitIndex,
    const QList<QList<UnitDef>>& categories,
    const QStringList& categoryNames) {

    ConversionResult out;

    if (categoryIndex < 0 || categoryIndex >= categories.size())
        return out;

    const QList<UnitDef>& defs = categories[categoryIndex];
    if (fromUnitIndex < 0 || toUnitIndex < 0 || fromUnitIndex >= defs.size() || toUnitIndex >= defs.size())
        return out;

    const QString fromU = defs[fromUnitIndex].name;
    const QString toU = defs[toUnitIndex].name;

    if (categoryNames[categoryIndex] == "Frequency (Hz)") {
        const auto isPeriodUnit = [](const QString& unit) { return unit == "s" || unit == "ms" || unit == "us"; };
        const auto periodToSeconds = [](double value, const QString& unit) {
            if (unit == "ms") return value * 1e-3;
            if (unit == "us") return value * 1e-6;
            return value;
        };
        const auto secondsToPeriod = [](double seconds, const QString& unit) {
            if (unit == "ms") return seconds * 1e3;
            if (unit == "us") return seconds * 1e6;
            return seconds;
        };

        double hzValue = 0.0;
        if (isPeriodUnit(fromU)) {
            const double seconds = periodToSeconds(fromValue, fromU);
            hzValue = (seconds == 0.0) ? std::numeric_limits<double>::infinity() : 1.0 / seconds;
        } else {
            hzValue = fromValue * defs[fromUnitIndex].toBase;
        }

        double result = 0.0;
        if (isPeriodUnit(toU)) {
            const double seconds = (hzValue == 0.0) ? std::numeric_limits<double>::infinity() : 1.0 / hzValue;
            result = secondsToPeriod(seconds, toU);
        } else {
            result = hzValue / defs[toUnitIndex].toBase;
        }

        const QString resultStr = formatResult(result);
        out.valid = true;
        out.resultText = resultStr + " " + toU;
        out.formulaText = QString("%1 %2 = %3 %4").arg(fromValue).arg(fromU).arg(resultStr).arg(toU);
        return out;
    }

    if (categoryNames[categoryIndex] == "Temperature") {
        double c = fromValue;
        if (fromU == "°F") c = (fromValue - 32.0) * 5.0 / 9.0;
        else if (fromU == "K") c = fromValue - 273.15;

        double result = c;
        if (toU == "°F") result = c * 9.0 / 5.0 + 32.0;
        else if (toU == "K") result = c + 273.15;

        out.valid = true;
        out.resultText = QString::number(result, 'g', 10) + " " + toU;
        out.formulaText = QString("%1 %2 → %3 %4").arg(fromValue).arg(fromU).arg(result).arg(toU);
        return out;
    }

    bool fromDbm = (fromU == "dBm");
    bool toDbm = (toU == "dBm");

    double baseVal = fromDbm ? 1e-3 * std::pow(10.0, fromValue / 10.0) : fromValue * defs[fromUnitIndex].toBase;
    double result = toDbm ? 10.0 * std::log10(baseVal / 1e-3) : baseVal / defs[toUnitIndex].toBase;

    const QString resultStr = formatResult(result);
    out.valid = true;
    out.resultText = resultStr + " " + toU;
    out.formulaText = QString("%1 %2 = %3 %4").arg(fromValue).arg(fromU).arg(resultStr).arg(toU);
    return out;
}

} // namespace Rheno::Core

