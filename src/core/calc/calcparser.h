#pragma once

#include "operations.h"
#include <QStringList>

namespace Rheno::Core {

QString bigToDisplayString(const BigDecimal& v);
QString bigToTokenString(const BigDecimal& v);
BigDecimal qStringToBig(const QString& s);

bool isBinaryOperatorToken(const QString& token);
int precedenceOf(const QString& token);

bool toRpn(const QStringList& tokens, QStringList* outRpn);
bool evalIntRpn(const QStringList& rpn, int base, int bits, long long* result);
bool evalDoubleRpn(const QStringList& rpn, double* result);
bool evalBigRpn(const QStringList& rpn, BigDecimal* result);

} // namespace Rheno::Core
