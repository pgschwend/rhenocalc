#include "calculatorpagecontroller.h"

#include "../calculatorpage.h"
#include "core/common/numberparse.h"
#include "ui/themecolors.h"

#include <QClipboard>
#include <QEvent>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QPushButton>
#include <QSettings>
#include <QWidget>

CalculatorPageController::CalculatorPageController(CalculatorPage* page) : m_page(page) {
}

void CalculatorPageController::initialize() {
    onWordWidthChanged(m_page->m_widthCombo->currentIndex());
    updateDisplay();
}

void CalculatorPageController::updateDisplay() {
    m_page->m_display->setText(m_page->m_engine.displayText());
    m_page->m_exprLabel->setText(m_page->m_engine.expressionText());
    const bool hexMode = (m_page->m_engine.base() == 16);
    for (auto* btn : m_page->m_hexBtns)
        btn->setEnabled(hexMode);
}

void CalculatorPageController::onBaseChanged(int index) {
    resetCeClearCycle();
    const int bases[] = {10, 16, 2, 8};
    m_page->m_engine.setBase(bases[index]);

    if (bases[index] != 10 && m_page->m_widthCombo->currentIndex() == 4)
        m_page->m_widthCombo->setCurrentIndex(3);

    updateDisplay();
}

void CalculatorPageController::onWordWidthChanged(int index) {
    resetCeClearCycle();
    const int widths[] = {8, 16, 32, 64};
    const bool precMode = (index == 4);
    m_page->m_engine.setBigMode(precMode);
    if (!precMode)
        m_page->m_engine.setWordBits(widths[index]);
    updateDisplay();
}

void CalculatorPageController::onDigitClicked() {
    auto* btn = qobject_cast<QPushButton*>(m_page->sender());
    if (btn)
        pressDigit(btn->text());
}

void CalculatorPageController::pressDigit(const QString& d) {
    resetCeClearCycle();
    m_page->m_engine.pressDigit(d);
    updateDisplay();
}

void CalculatorPageController::onOperatorClicked() {
    auto* btn = qobject_cast<QPushButton*>(m_page->sender());
    if (!btn)
        return;

    QString op = btn->text();
    if (op == "÷") op = "/";
    if (op == "×") op = "*";
    pressOperator(op);
}

void CalculatorPageController::pressOperator(const QString& op) {
    resetCeClearCycle();
    m_page->m_engine.pressOperator(op);
    updateDisplay();
}

void CalculatorPageController::onEqualsClicked() {
    resetCeClearCycle();
    m_page->m_engine.equals();
    updateDisplay();
}

void CalculatorPageController::onClearEntryOrAllClicked() {
    const bool hasVisibleInput = (m_page->m_engine.displayText() != "0") || !m_page->m_engine.expressionText().isEmpty();
    if (!m_page->m_ceEntryCleared && hasVisibleInput) {
        m_page->m_engine.clearEntry();
        m_page->m_ceEntryCleared = true;
    } else {
        m_page->m_engine.clearAll();
        m_page->m_ceEntryCleared = false;
    }
    updateDisplay();
}

void CalculatorPageController::onClearClicked() {
    resetCeClearCycle();
    m_page->m_engine.clearAll();
    updateDisplay();
}

void CalculatorPageController::onBackspaceClicked() {
    resetCeClearCycle();
    m_page->m_engine.backspace();
    updateDisplay();
}

void CalculatorPageController::onNegateClicked() {
    resetCeClearCycle();
    m_page->m_engine.negate();
    updateDisplay();
}

void CalculatorPageController::onPiClicked() {
    resetCeClearCycle();
    auto* btn = qobject_cast<QPushButton*>(m_page->sender());
    if (!btn)
        return;

    const QString op = btn->objectName();

    if (op == "π") {
        m_page->m_engine.setPi();
    } else if (op == "e") {
        m_page->m_engine.setEuler();
    } else if (op == "sin" || op == "asin" || op == "LOGXY") {
        m_page->m_engine.applyBitwiseOrFunction(op);
    }

    updateDisplay();
}

void CalculatorPageController::onSecondFuncToggled() {
    m_page->m_secondActive = !m_page->m_secondActive;
    updateSecondFuncButtons();
}

void CalculatorPageController::updateSecondFuncButtons() {
    if (m_page->m_secondFuncBtn) {
        if (m_page->m_secondActive)
            m_page->m_secondFuncBtn->setStyleSheet(Rheno::UI::calcSecondFuncButtonActive(m_page->m_isDark));
        else
            m_page->m_secondFuncBtn->setStyleSheet(Rheno::UI::calcSecondFuncButton(m_page->m_isDark));
    }

    if (m_page->m_secondActive) {
        if (m_page->m_piBtn) { m_page->m_piBtn->setText("logᵧx"); m_page->m_piBtn->setObjectName("LOGXY"); }
        if (m_page->m_sqBtn) { m_page->m_sqBtn->setText("xʸ"); m_page->m_sqBtn->setObjectName("POW"); }
        if (m_page->m_sqrtBtn) { m_page->m_sqrtBtn->setText("ʸ√x"); m_page->m_sqrtBtn->setObjectName("NROOT"); }
        if (m_page->m_eBtn) { m_page->m_eBtn->setText("sin"); m_page->m_eBtn->setObjectName("sin"); }
        if (m_page->m_logBtn) { m_page->m_logBtn->setText("cos"); m_page->m_logBtn->setObjectName("cos"); }
        if (m_page->m_lnBtn) { m_page->m_lnBtn->setText("tan"); m_page->m_lnBtn->setObjectName("tan"); }
        if (m_page->m_msBtn) { m_page->m_msBtn->setText("sin⁻¹"); m_page->m_msBtn->setObjectName("asin"); }
        if (m_page->m_mrBtn) { m_page->m_mrBtn->setText("cos⁻¹"); m_page->m_mrBtn->setObjectName("acos"); }
        if (m_page->m_mcBtn) { m_page->m_mcBtn->setText("tan⁻¹"); m_page->m_mcBtn->setObjectName("atan"); }
    } else {
        if (m_page->m_piBtn) { m_page->m_piBtn->setText("π"); m_page->m_piBtn->setObjectName("π"); }
        if (m_page->m_sqBtn) { m_page->m_sqBtn->setText("x²"); m_page->m_sqBtn->setObjectName("SQ"); }
        if (m_page->m_sqrtBtn) { m_page->m_sqrtBtn->setText("√x"); m_page->m_sqrtBtn->setObjectName("SQRT"); }
        if (m_page->m_eBtn) { m_page->m_eBtn->setText("e"); m_page->m_eBtn->setObjectName("e"); }
        if (m_page->m_logBtn) { m_page->m_logBtn->setText("log"); m_page->m_logBtn->setObjectName("log"); }
        if (m_page->m_lnBtn) { m_page->m_lnBtn->setText("ln"); m_page->m_lnBtn->setObjectName("ln"); }
        if (m_page->m_msBtn) { m_page->m_msBtn->setText("MS"); m_page->m_msBtn->setObjectName("MS"); }
        if (m_page->m_mrBtn) { m_page->m_mrBtn->setText("MR"); m_page->m_mrBtn->setObjectName("MR"); }
        if (m_page->m_mcBtn) { m_page->m_mcBtn->setText("MC"); m_page->m_mcBtn->setObjectName("MC"); }
    }
}

void CalculatorPageController::onBitwiseClicked() {
    resetCeClearCycle();
    auto* btn = qobject_cast<QPushButton*>(m_page->sender());
    if (!btn)
        return;

    m_page->m_engine.applyBitwiseOrFunction(btn->objectName());
    updateDisplay();
}

void CalculatorPageController::resetCeClearCycle() {
    m_page->m_ceEntryCleared = false;
}

void CalculatorPageController::applyTheme(bool dark) {
    m_page->m_isDark = dark;

    const QString numS = Rheno::UI::calcNumButton(dark);
    const QString opS = Rheno::UI::calcOpButton(dark);
    const QString bitS = Rheno::UI::calcBitButton(dark);
    const QString funcS = Rheno::UI::calcFuncButton(dark);
    const QString hexS = Rheno::UI::calcHexButton(dark);
    const QString eqS = Rheno::UI::calcEqButton(dark);
    const QString clrS = Rheno::UI::calcClearButton(dark);
    const QString dispS = Rheno::UI::calcDisplayStyle(dark);
    const QString exprS = Rheno::UI::calcExprStyle(dark);
    const QString hintS = Rheno::UI::calcHintStyle(dark);

    m_page->m_display->setStyleSheet(dispS);
    m_page->m_exprLabel->setStyleSheet(exprS);
    m_page->m_hintLabel->setStyleSheet(hintS);

    for (auto* b : m_page->m_numBtns) b->setStyleSheet(numS);
    for (auto* b : m_page->m_opBtns) b->setStyleSheet(opS);
    for (auto* b : m_page->m_bitOpBtns) b->setStyleSheet(bitS);
    for (auto* b : m_page->m_funcBtns) b->setStyleSheet(funcS);
    for (auto* b : m_page->m_clearBtns) b->setStyleSheet(clrS);
    for (auto* b : m_page->m_hexBtns) b->setStyleSheet(hexS);
    if (m_page->m_eqBtn) m_page->m_eqBtn->setStyleSheet(eqS);

    updateSecondFuncButtons();
}

void CalculatorPageController::onShow() {
    m_page->setFocus();
}

bool CalculatorPageController::onKeyPress(QKeyEvent* event) {
    const int key = event->key();
    const Qt::KeyboardModifiers mod = event->modifiers();

    if (mod == Qt::ControlModifier && key == Qt::Key_C) {
        if (auto* clipboard = QGuiApplication::clipboard())
            clipboard->setText(m_page->m_engine.displayText());
        return true;
    }

    if (mod == Qt::ControlModifier && key == Qt::Key_V) {
        auto* clipboard = QGuiApplication::clipboard();
        if (!clipboard)
            return true;

        QString text = Rheno::Core::sanitizePastedNumber(clipboard->text(), m_page->m_engine.base());
        if (text.isEmpty())
            return true;

        bool negative = false;
        if (!Rheno::Core::extractSignPrefix(&text, &negative))
            return true;

        if (!Rheno::Core::isValidForBase(text, m_page->m_engine.base(), true))
            return true;

        resetCeClearCycle();
        m_page->m_engine.clearEntry();

        for (const QChar ch : text) {
            if (ch == ',')
                m_page->m_engine.pressDigit(".");
            else
                m_page->m_engine.pressDigit(QString(ch));
        }

        if (negative)
            m_page->m_engine.negate();

        updateDisplay();
        return true;
    }

    if (key == Qt::Key_Period || key == Qt::Key_Comma) {
        pressDigit(".");
        return true;
    }

    if (key == Qt::Key_ParenLeft) {
        pressDigit("(");
        return true;
    }
    if (key == Qt::Key_ParenRight) {
        pressDigit(")");
        return true;
    }

    const bool hasBlockedModifier = (mod & (Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier));
    if (key >= Qt::Key_0 && key <= Qt::Key_9 && !hasBlockedModifier) {
        const QString d = QString::number(key - Qt::Key_0);
        if (m_page->m_engine.base() == 2 && d.toInt() > 1) { return false; }
        if (m_page->m_engine.base() == 8 && d.toInt() > 7) { return false; }
        pressDigit(d);
        return true;
    }

    if (m_page->m_engine.base() == 16 && (mod == Qt::NoModifier || mod == Qt::ShiftModifier)) {
        if (key >= Qt::Key_A && key <= Qt::Key_F) {
            pressDigit(QString(QChar('A' + (key - Qt::Key_A))));
            return true;
        }
    }

    switch (key) {
    case Qt::Key_Plus: pressOperator("+"); return true;
    case Qt::Key_Minus: pressOperator("-"); return true;
    case Qt::Key_Asterisk: pressOperator("*"); return true;
    case Qt::Key_Slash: pressOperator("/"); return true;
    case Qt::Key_Percent: pressOperator("MOD"); return true;

    case Qt::Key_Return:
    case Qt::Key_Enter:
    case Qt::Key_Equal: onEqualsClicked(); return true;

    case Qt::Key_Escape: {
        if (!m_page->m_engine.isClearState()) {
            resetCeClearCycle();
            m_page->m_engine.clearAllAndMemory();
            updateDisplay();
            return true;
        }
        if (QWidget* mainWin = m_page->window()) {
            QSettings settings("RhenoCalc", "RhenoCalc");
            if (settings.value("closeWithEscCheck", false).toBool())
                mainWin->close();
        }
        return true;
    }
    case Qt::Key_Delete: onClearClicked(); return true;
    case Qt::Key_Backspace: onBackspaceClicked(); return true;

    case Qt::Key_Ampersand:
        m_page->m_engine.applyBitwiseOrFunction("AND");
        updateDisplay();
        return true;
    case Qt::Key_Bar:
        m_page->m_engine.applyBitwiseOrFunction("OR");
        updateDisplay();
        return true;
    case Qt::Key_AsciiCircum:
        m_page->m_engine.applyBitwiseOrFunction("XOR");
        updateDisplay();
        return true;
    case Qt::Key_AsciiTilde:
        m_page->m_engine.applyBitwiseOrFunction("NOT");
        updateDisplay();
        return true;
    case Qt::Key_Less:
        m_page->m_engine.applyBitwiseOrFunction("LSL");
        updateDisplay();
        return true;
    case Qt::Key_Greater:
        m_page->m_engine.applyBitwiseOrFunction("LSR");
        updateDisplay();
        return true;
    case Qt::Key_N:
        if (mod == Qt::NoModifier) { onNegateClicked(); return true; }
        break;

    case Qt::Key_D:
        if (mod == Qt::AltModifier) { m_page->m_baseCombo->setCurrentIndex(0); return true; }
        break;
    case Qt::Key_H:
        if (mod == Qt::AltModifier) { m_page->m_baseCombo->setCurrentIndex(1); return true; }
        break;
    case Qt::Key_B:
        if (mod == Qt::AltModifier) { m_page->m_baseCombo->setCurrentIndex(2); return true; }
        break;
    case Qt::Key_O:
        if (mod == Qt::AltModifier) { m_page->m_baseCombo->setCurrentIndex(3); return true; }
        break;

    case Qt::Key_1:
        if (mod == Qt::AltModifier) { m_page->m_widthCombo->setCurrentIndex(0); return true; }
        break;
    case Qt::Key_2:
        if (mod == Qt::AltModifier) { m_page->m_widthCombo->setCurrentIndex(1); return true; }
        break;
    case Qt::Key_3:
        if (mod == Qt::AltModifier) { m_page->m_widthCombo->setCurrentIndex(2); return true; }
        break;
    case Qt::Key_4:
        if (mod == Qt::AltModifier) { m_page->m_widthCombo->setCurrentIndex(3); return true; }
        break;
    case Qt::Key_5:
        if (mod == Qt::AltModifier) { m_page->m_widthCombo->setCurrentIndex(4); return true; }
        break;

    default:
        break;
    }

    return false;
}

bool CalculatorPageController::onEventFilter(QObject* watched, QEvent* event) {
    if ((watched == m_page->m_baseCombo || watched == m_page->m_widthCombo) && event->type() == QEvent::KeyPress) {
        auto* keyEvent = dynamic_cast<QKeyEvent*>(event);
        if (!keyEvent)
            return false;
        const int key = keyEvent->key();
        const Qt::KeyboardModifiers mod = keyEvent->modifiers();

        if (key == Qt::Key_Up || key == Qt::Key_Down ||
            key == Qt::Key_Return || key == Qt::Key_Enter ||
            key == Qt::Key_Space) {
            return false;
        }

        if (key == Qt::Key_Escape || key == Qt::Key_Delete)
            return false;

        if (mod & Qt::AltModifier) {
            if (key == Qt::Key_Tab || key == Qt::Key_Backtab)
                return false;
            return onKeyPress(keyEvent);
        }

        return onKeyPress(keyEvent);
    }

    return false;
}


