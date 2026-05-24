#pragma once

#include <QPalette>
#include <QString>

namespace ThemeColors {

QString applyQssColors(QString qssTemplate, bool dark);
QPalette applicationPalette(bool dark);

QString statusBarStyle(bool dark);
QString themeToggleButtonStyle(bool dark);

QString calcNumButton(bool dark);
QString calcOpButton(bool dark);
QString calcBitButton(bool dark);
QString calcFuncButton(bool dark);
QString calcHexButton(bool dark);
QString calcEqButton(bool dark);
QString calcClearButton(bool dark);
QString calcDisplayStyle(bool dark);
QString calcExprStyle(bool dark);
QString calcHintStyle(bool dark);

QString baseBitButtonStyle(bool dark, bool on);
QString baseGroupStyle(bool dark);
QString baseFieldLabelStyle(bool dark);
QString baseEditStyle(bool dark);
QString baseValueStyle(bool dark);

QString unitGroupStyle(bool dark);
QString unitFieldStyle(bool dark);
QString unitResultStyle(bool dark);
QString unitTitleStyle(bool dark);
QString unitFormulaStyle(bool dark);

} // namespace ThemeColors

