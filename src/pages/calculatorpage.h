#pragma once
#include "core/calculatorcore.h"
#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QButtonGroup>
#include <QKeyEvent>
#include <QList>
#include <cmath>
#include <limits>

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
    QPushButton* makeBtn(const QString& text, const QString& style = "");
    QString toBaseString(long long val);
    long long fromBaseString(const QString& s);
    long long maskToWidth(long long val);
    QString formatDouble(double val);

    QLineEdit*   m_display;
    QLabel*      m_exprLabel;
    QLabel*      m_hintLabel;
    QLabel*      m_baseLabel;
    QLabel*      m_wordLabel;
    QComboBox*   m_baseCombo;
    QComboBox*   m_widthCombo;

    // Hex digit buttons
    QPushButton* m_hexBtns[6]; // A-F

    // Button groups for theming
    QList<QPushButton*> m_numBtns;
    QList<QPushButton*> m_opBtns;
    QList<QPushButton*> m_bitOpBtns;
    QList<QPushButton*> m_funcBtns;
    QList<QPushButton*> m_clearBtns;
    QPushButton*        m_eqBtn = nullptr;
    bool                m_ceEntryCleared = false;

    // 2nd function toggle
    bool                m_secondActive = false;
    QPushButton*        m_secondFuncBtn = nullptr;
    QPushButton*        m_piBtn = nullptr;      // π ↔ sin
    QPushButton*        m_sqBtn = nullptr;      // x² ↔ cos
    QPushButton*        m_sqrtBtn = nullptr;    // √x ↔ tan
    QPushButton*        m_eBtn = nullptr;       // e ↔ asin
    QPushButton*        m_logBtn = nullptr;     // log ↔ acos
    QPushButton*        m_lnBtn = nullptr;      // ln ↔ atan
    QPushButton*        m_msBtn = nullptr;      // MS ↔ sinh
    QPushButton*        m_mrBtn = nullptr;      // MR ↔ cosh
    QPushButton*        m_mcBtn = nullptr;      // MC ↔ tanh
    bool                m_isDark = true;

    CalculatorCore::CalculatorEngine m_engine;
};
