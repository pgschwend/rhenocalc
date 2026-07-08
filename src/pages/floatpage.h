#pragma once

#include <QWidget>

class QLabel;
class QLineEdit;
class QComboBox;

class FloatPage : public QWidget {
    Q_OBJECT

public:
    explicit FloatPage(QWidget* parent = nullptr);
    void applyTheme(bool dark);

private:
    void setupUI();
    void floatToBinary();
    void binaryToFloat();

    QLabel* m_titleLabel = nullptr;

    // Float to Binary section
    QComboBox* m_floatTypeCombo = nullptr;
    QLineEdit* m_floatInput = nullptr;
    QLabel* m_signLabel = nullptr;
    QLabel* m_exponentLabel = nullptr;
    QLabel* m_mantissaLabel = nullptr;
    QLabel* m_binaryLabel = nullptr;
    QLabel* m_hexLabel = nullptr;

    // Binary to Float section
    QComboBox* m_binTypeCombo = nullptr;
    QLineEdit* m_binaryInput = nullptr;
    QLabel* m_binSignLabel = nullptr;
    QLabel* m_binExponentLabel = nullptr;
    QLabel* m_binMantissaLabel = nullptr;
    QLabel* m_floatResultLabel = nullptr;
    QLabel* m_hexResultLabel = nullptr;

    bool m_isDark = true;
};
