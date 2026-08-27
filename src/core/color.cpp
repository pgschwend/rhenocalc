#include "color.h"

#include <QtMath>

namespace Rheno::Core {

bool parseHexColor(QString text, QColor* outColor) {
    text = text.trimmed();
    if (!text.startsWith('#'))
        text.prepend('#');
    const QColor c(text);
    if (!c.isValid())
        return false;
    *outColor = c;
    return true;
}

namespace {
int clampedIntOrDefault(const QString& text, int lo, int hi, int fallback) {
    bool ok = false;
    const int v = text.toInt(&ok);
    return ok ? qBound(lo, v, hi) : fallback;
}
} // namespace

QColor fromRgbText(const QString& r, const QString& g, const QString& b, const QString& a) {
    return QColor(
        clampedIntOrDefault(r, 0, 255, 0),
        clampedIntOrDefault(g, 0, 255, 0),
        clampedIntOrDefault(b, 0, 255, 0),
        clampedIntOrDefault(a, 0, 255, 255));
}

QColor fromHslText(const QString& h, const QString& s, const QString& l, const QString& a) {
    return QColor::fromHsl(
        clampedIntOrDefault(h, 0, 359, 0),
        clampedIntOrDefault(s, 0, 255, 0),
        clampedIntOrDefault(l, 0, 255, 0),
        clampedIntOrDefault(a, 0, 255, 255));
}

QString previewText(const QColor& color) {
    return QString("RGB(%1, %2, %3)  |  HSL(%4, %5%, %6%)")
        .arg(color.red()).arg(color.green()).arg(color.blue())
        .arg(qMax(0, color.hslHue()))
        .arg(qRound(color.hslSaturationF() * 100))
        .arg(qRound(color.lightnessF() * 100));
}

} // namespace Rheno::Core

