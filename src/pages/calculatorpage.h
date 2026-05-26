#pragma once
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
    void onClearClicked();
    void onBackspaceClicked();
    void onPiClicked();
    void onBitwiseClicked();
    void onWordWidthChanged(int index);
    void onBaseChanged(int index);
    void onNegateClicked();

protected:
    void keyPressEvent(QKeyEvent* event) override;

private:
    void setupUI();
    void updateDisplay();
    void pressDigit(const QString& d);
    void pressOperator(const QString& op);
    void applyResult(long long result);
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

    long long m_current       = 0;
    long long m_accumulator   = 0;
    double    m_currentDouble = 0.0;
    double    m_accumulatorDouble = 0.0;
    QString   m_pendingOp;
    QString   m_inputString;       // used in float mode to build input character-by-character
    bool      m_newInput      = true;
    bool      m_hasDecimal    = false;
    bool      m_floatMode     = false;
    int       m_base          = 10;
    int       m_wordBits      = 32;
};
