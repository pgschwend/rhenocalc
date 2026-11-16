#pragma once

#include <QLocale>
#include <QString>

namespace Rheno::Core {

enum class EmptyNumberPolicy {
    Invalid,
    Zero
};

// `locale` defaults to the OS locale and is otherwise only overridden by tests.
bool tryParseLocalizedDouble(const QString& text, double* value, EmptyNumberPolicy emptyPolicy = EmptyNumberPolicy::Invalid,
                              const QLocale& locale = QLocale::system());

QString sanitizePastedNumber(QString text, int base);
bool extractSignPrefix(QString* text, bool* negative);
bool isValidForBase(const QString& text, int base, bool allowDecimalPoint = false);

} // namespace Rheno::Core

