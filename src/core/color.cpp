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
int clampedIntOrDefault(const QString& text, int lo, int hi, int fallback, int base = 10) {
    bool ok = false;
    const int v = text.toInt(&ok, base);
    return ok ? qBound(lo, v, hi) : fallback;
}
} // namespace

QColor fromRgbText(const QString& r, const QString& g, const QString& b, const QString& a, int base) {
    return QColor(
        clampedIntOrDefault(r, 0, 255, 0, base),
        clampedIntOrDefault(g, 0, 255, 0, base),
        clampedIntOrDefault(b, 0, 255, 0, base),
        clampedIntOrDefault(a, 0, 255, 255, base));
}

QColor fromHslText(const QString& h, const QString& s, const QString& l, const QString& a) {
    return QColor::fromHsl(
        clampedIntOrDefault(h, 0, 359, 0),
        clampedIntOrDefault(s, 0, 255, 0),
        clampedIntOrDefault(l, 0, 255, 0),
        clampedIntOrDefault(a, 0, 255, 255));
}

QString previewText(const QColor& color, bool hexRgb) {
    auto hexByte = [](int v) { return QString("%1").arg(v, 2, 16, QChar('0')).toUpper(); };
    const QString rgbPart = hexRgb
        ? QString("RGB(0x%1, 0x%2, 0x%3)").arg(hexByte(color.red()), hexByte(color.green()), hexByte(color.blue()))
        : QString("RGB(%1, %2, %3)").arg(color.red()).arg(color.green()).arg(color.blue());

    return QString("%1  |  HSL(%2, %3%, %4%)")
        .arg(rgbPart)
        .arg(qMax(0, color.hslHue()))
        .arg(qRound(color.hslSaturationF() * 100))
        .arg(qRound(color.lightnessF() * 100));
}

} // namespace Rheno::Core

