#include "calculatorpage.h"

#include "calculator/calculatorpagecontroller.h"
#include "calculator/calculatorpageuibuilder.h"

CalculatorPage::CalculatorPage(QWidget* parent) : QWidget(parent) {
    Rheno::Ui::buildCalculatorPageUi(this);
    m_controller = std::make_unique<CalculatorPageController>(this);
    m_controller->initialize();
}

CalculatorPage::~CalculatorPage() = default;

void CalculatorPage::updateDisplay() {
    m_controller->updateDisplay();
}

void CalculatorPage::onBaseChanged(int index) {
    m_controller->onBaseChanged(index);
}

void CalculatorPage::onWordWidthChanged(int index) {
    m_controller->onWordWidthChanged(index);
}

void CalculatorPage::onDigitClicked() {
    m_controller->onDigitClicked();
}

void CalculatorPage::pressDigit(const QString& d) {
    m_controller->pressDigit(d);
}

void CalculatorPage::onOperatorClicked() {
    m_controller->onOperatorClicked();
}

void CalculatorPage::pressOperator(const QString& op) {
    m_controller->pressOperator(op);
}

void CalculatorPage::onEqualsClicked() {
    m_controller->onEqualsClicked();
}

void CalculatorPage::onClearEntryOrAllClicked() {
    m_controller->onClearEntryOrAllClicked();
}

void CalculatorPage::onClearClicked() {
    m_controller->onClearClicked();
}

void CalculatorPage::onBackspaceClicked() {
    m_controller->onBackspaceClicked();
}

void CalculatorPage::onNegateClicked() {
    m_controller->onNegateClicked();
}

void CalculatorPage::onPiClicked() {
    m_controller->onPiClicked();
}

void CalculatorPage::onSecondFuncToggled() {
    m_controller->onSecondFuncToggled();
}

void CalculatorPage::updateSecondFuncButtons() {
    m_controller->updateSecondFuncButtons();
}

void CalculatorPage::onBitwiseClicked() {
    m_controller->onBitwiseClicked();
}

void CalculatorPage::resetCeClearCycle() {
    m_controller->resetCeClearCycle();
}

void CalculatorPage::applyTheme(bool dark) {
    m_controller->applyTheme(dark);
}

void CalculatorPage::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    m_controller->onShow();
}

void CalculatorPage::keyPressEvent(QKeyEvent* event) {
    if (m_controller->onKeyPress(event))
        return;
    QWidget::keyPressEvent(event);
}

bool CalculatorPage::eventFilter(QObject* watched, QEvent* event) {
    if (m_controller->onEventFilter(watched, event))
        return true;
    return QWidget::eventFilter(watched, event);
}

