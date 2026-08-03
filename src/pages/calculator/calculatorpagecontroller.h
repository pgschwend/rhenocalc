#pragma once

class CalculatorPage;
class QObject;
class QEvent;
class QKeyEvent;

#include <QString>

class CalculatorPageController {
public:
    explicit CalculatorPageController(CalculatorPage* page);

    void initialize();

    void updateDisplay();
    void onBaseChanged(int index);
    void onWordWidthChanged(int index);
    void onDigitClicked();
    void pressDigit(const QString& d);
    void onOperatorClicked();
    void pressOperator(const QString& op);
    void onEqualsClicked();
    void onClearEntryOrAllClicked();
    void onClearClicked();
    void onBackspaceClicked();
    void onNegateClicked();
    void onPiClicked();
    void onSecondFuncToggled();
    void updateSecondFuncButtons();
    void onBitwiseClicked();
    void resetCeClearCycle();

    void applyTheme(bool dark);
    void onShow();
    bool onKeyPress(QKeyEvent* event);
    bool onEventFilter(QObject* watched, QEvent* event);

private:
    CalculatorPage* m_page = nullptr;
};



