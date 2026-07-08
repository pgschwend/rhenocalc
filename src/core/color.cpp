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

QColor fromRgbText(const QString& r, const QString& g, const QString& b, const QString& a) {
    return QColor(r.toInt(), g.toInt(), b.toInt(), a.toInt());
}

QColor fromHslText(const QString& h, const QString& s, const QString& l, const QString& a) {
    return QColor::fromHsl(h.toInt(), s.toInt(), l.toInt(), a.toInt());
}

QString previewText(const QColor& color) {
    return QString("RGB(%1, %2, %3)  |  HSL(%4, %5%, %6%)")
        .arg(color.red()).arg(color.green()).arg(color.blue())
        .arg(qMax(0, color.hslHue()))
        .arg(qRound(color.hslSaturationF() * 100))
        .arg(qRound(color.lightnessF() * 100));
}

} // namespace Rheno::Core

