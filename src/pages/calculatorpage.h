#pragma once
#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QButtonGroup>
#include <QKeyEvent>

class CalculatorPage : public QWidget {
    Q_OBJECT
public:
    explicit CalculatorPage(QWidget* parent = nullptr);

private slots:
    void onDigitClicked();
    void onOperatorClicked();
    void onEqualsClicked();
    void onClearClicked();
    void onBackspaceClicked();
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

    QLineEdit*   m_display;
    QLabel*      m_exprLabel;
    QComboBox*   m_baseCombo;
    QComboBox*   m_widthCombo;

    // Hex digit buttons
    QPushButton* m_hexBtns[6]; // A-F

    long long m_current    = 0;
    long long m_accumulator= 0;
    QString   m_pendingOp;
    bool      m_newInput   = true;
    bool      m_hasDecimal = false;
    double    m_currentDouble = 0.0;
    bool      m_floatMode  = false;
    int       m_base       = 10;
    int       m_wordBits   = 32;
};

