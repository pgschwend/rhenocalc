#pragma once

#include <QString>

namespace Rheno::Core {

QString formatEngineeringValue(double value, const QString& unit);
QString formatMoneyLocalized(double value, int decimals = 2);

} // namespace Rheno::Core


