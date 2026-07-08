#pragma once

#include <QStringList>

namespace Rheno::Core {

unsigned long long applyMask(unsigned long long value, int bits);
QString formatBinarySpaced(unsigned long long value, int bits);
bool tryParse(const QString& text, int base, unsigned long long& outValue);

long long signedValue(unsigned long long value, int bits);
QString hexWithPadding(unsigned long long value, int bits);
QString float32String(unsigned long long value);

QString byteHex(unsigned long long value, int byteIndex);
QString byteTooltip(unsigned long long value, int byteIndex);

} // namespace Rheno::Core

