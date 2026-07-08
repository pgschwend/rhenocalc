#pragma once

#include "core/calculatorcore.h"
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QKeyEvent>
#include <QList>

class CalculatorPage : public QWidget {
    Q_OBJECT
public:
    explicit CalculatorPage(QWidget* parent = nullptr);
    void applyTheme(bool dark);

private slots:
    void onDigitClicked();
    void onOperatorClicked();
    void onEqualsClicked();
    void onClearEntryOrAllClicked();
    void onClearClicked();
    void onBackspaceClicked();
    void onPiClicked();
    void onBitwiseClicked();
    void onWordWidthChanged(int index);
    void onBaseChanged(int index);
    void onNegateClicked();
    void onSecondFuncToggled();

protected:
    void keyPressEvent(QKeyEvent* event) override;

private:
    void setupUI();
    void updateDisplay();
    void resetCeClearCycle();
    void pressDigit(const QString& d);
    void pressOperator(const QString& op);
    void updateSecondFuncButtons();
    QPushButton* makeBtn(const QString& text);

    QLineEdit*   m_display = nullptr;
    QLabel*      m_exprLabel = nullptr;
    QLabel*      m_hintLabel = nullptr;
    QLabel*      m_baseLabel = nullptr;
    QLabel*      m_wordLabel = nullptr;
    QComboBox*   m_baseCombo = nullptr;
    QComboBox*   m_widthCombo = nullptr;

    QPushButton* m_hexBtns[6]{}; // A-F

    // Button groups for theming
    QList<QPushButton*> m_numBtns;
    QList<QPushButton*> m_opBtns;
    QList<QPushButton*> m_bitOpBtns;
    QList<QPushButton*> m_funcBtns;
    QList<QPushButton*> m_clearBtns;
    QPushButton*        m_eqBtn = nullptr;
    bool                m_ceEntryCleared = false;

    // 2nd function toggle buttons
    bool                m_secondActive = false;
    QPushButton*        m_secondFuncBtn = nullptr;
    QPushButton*        m_piBtn = nullptr;
    QPushButton*        m_sqBtn = nullptr;
    QPushButton*        m_sqrtBtn = nullptr;
    QPushButton*        m_eBtn = nullptr;
    QPushButton*        m_logBtn = nullptr;
    QPushButton*        m_lnBtn = nullptr;
    QPushButton*        m_msBtn = nullptr;
    QPushButton*        m_mrBtn = nullptr;
    QPushButton*        m_mcBtn = nullptr;
    bool                m_isDark = true;

    Rheno::Core::CalculatorEngine m_engine;
};
