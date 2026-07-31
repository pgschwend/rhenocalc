#pragma once

#include <QString>

namespace Rheno::Core {

enum class EmptyNumberPolicy {
    Invalid,
    Zero
};

bool tryParseLocalizedDouble(const QString& text, double* value, EmptyNumberPolicy emptyPolicy = EmptyNumberPolicy::Invalid);

QString sanitizePastedNumber(QString text, int base);
bool extractSignPrefix(QString* text, bool* negative);
bool isValidForBase(const QString& text, int base, bool allowDecimalPoint = false);

} // namespace Rheno::Core

