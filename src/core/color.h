#pragma once

#include <QColor>
#include <QString>

namespace Rheno::Core {

bool parseHexColor(QString text, QColor* outColor);
QColor fromRgbText(const QString& r, const QString& g, const QString& b, const QString& a);
QColor fromHslText(const QString& h, const QString& s, const QString& l, const QString& a);
QString previewText(const QColor& color);

} // namespace Rheno::Core

