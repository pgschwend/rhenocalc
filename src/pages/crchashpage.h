#pragma once

#include <QWidget>

class QComboBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QPushButton;

class CrcHashPage : public QWidget {
    Q_OBJECT

public:
    explicit CrcHashPage(QWidget* parent = nullptr);
    void applyTheme(bool dark);

private slots:
    void recalculate();
    void copyResult();

private:
    void setupUI();

    QLabel* m_titleLabel = nullptr;
    QGroupBox* m_inputGroup = nullptr;
    QGroupBox* m_outputGroup = nullptr;

    QLabel* m_algoLabel = nullptr;
    QComboBox* m_algoCombo = nullptr;
    QPlainTextEdit* m_inputEdit = nullptr;
    QLineEdit* m_outputEdit = nullptr;
    QPushButton* m_copyBtn = nullptr;
    QLabel* m_statusLabel = nullptr;
};

