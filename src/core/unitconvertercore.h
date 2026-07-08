#pragma once

#include <QList>
#include <QString>
#include <QStringList>

namespace Rheno::Core {

struct UnitDef {
    QString name;
    double toBase = 1.0;
};

struct ConversionResult {
    bool valid = false;
    QString resultText;
    QString formulaText;
};

QList<QList<UnitDef>> defaultCategories();
QStringList defaultCategoryNames();
ConversionResult convert(
    double fromValue,
    int categoryIndex,
    int fromUnitIndex,
    int toUnitIndex,
    const QList<QList<UnitDef>>& categories,
    const QStringList& categoryNames);

} // namespace Rheno::Core

