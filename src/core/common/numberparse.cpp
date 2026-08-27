#include "numberparse.h"

#include <QLocale>
#include <QRegularExpression>

namespace Rheno::Core {

bool tryParseLocalizedDouble(const QString& text, double* value, EmptyNumberPolicy emptyPolicy, const QLocale& locale) {
    if (!value)
        return false;

    QString trimmed = text.trimmed();
    if (trimmed.isEmpty()) {
        if (emptyPolicy == EmptyNumberPolicy::Zero) {
            *value = 0.0;
            return true;
        }
        return false;
    }

    bool ok = false;
    double parsed = locale.toDouble(trimmed, &ok);
    if (!ok && trimmed.contains(',')) {
        // A lone ',' the locale couldn't parse is most likely a European-style decimal
        // separator (e.g. "1,5") -- *unless* the locale itself groups thousands with ','
        // (e.g. en-US) and the text matches that grouping pattern (e.g. "1,234"), in
        // which case blindly guessing "decimal" would silently turn 1234 into 1.234
        // instead of correctly rejecting the unparseable input.
        static const QRegularExpression groupedIntegerPattern(R"(^[+-]?\d{1,3}(,\d{3})+(\.\d+)?$)");
        const bool localeGroupsWithComma = (locale.groupSeparator() == QChar(','));
        if (!(localeGroupsWithComma && groupedIntegerPattern.match(trimmed).hasMatch())) {
            QString normalized = trimmed;
            parsed = normalized.replace(',', '.').toDouble(&ok);
        }
    }
    if (!ok)
        return false;

    *value = parsed;
    return true;
}

QString sanitizePastedNumber(QString text, int base) {
    text = text.trimmed().toUpper();
    text.remove(' ');
    text.remove('_');
    text.remove('\'');

    if (base == 16 && text.startsWith("0X")) text.remove(0, 2);
    if (base == 2  && text.startsWith("0B")) text.remove(0, 2);
    if (base == 8  && text.startsWith("0O")) text.remove(0, 2);

    return text;
}

bool extractSignPrefix(QString* text, bool* negative) {
    if (!text || !negative)
        return false;

    *negative = false;
    if (text->startsWith('-')) {
        *negative = true;
        text->remove(0, 1);
    } else if (text->startsWith('+')) {
        text->remove(0, 1);
    }

    return !text->isEmpty();
}

bool isValidForBase(const QString& text, int base, bool allowDecimalPoint) {
    bool hasValidDigit = false;

    for (const QChar ch : text) {
        // Check against plain ASCII '0'-'9' explicitly rather than QChar::isDigit(),
        // which also accepts non-ASCII decimal-digit scripts that toLongLong()/
        // toDouble() don't understand as numeric digits.
        if (ch.unicode() >= '0' && ch.unicode() <= '9') {
            const int v = ch.unicode() - '0';
            if ((base == 2 && v > 1) || (base == 8 && v > 7))
                return false;
            hasValidDigit = true;
            continue;
        }

        if (base == 16 && ((ch >= 'A' && ch <= 'F') || (ch >= 'a' && ch <= 'f'))) {
            hasValidDigit = true;
            continue;
        }

        if (allowDecimalPoint && base == 10 && (ch == '.' || ch == ','))
            continue;

        return false;
    }

    return hasValidDigit;
}

} // namespace Rheno::Core

