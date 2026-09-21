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

    int algorithmIndex() const;
    void setAlgorithmIndex(int index);
    int inputFormatIndex() const;
    void setInputFormatIndex(int index);

private slots:
    void recalculate();
    void copyResult();

private:
    void setupUI();

    QLabel* m_titleLabel = nullptr;
    QGroupBox* m_inputGroup = nullptr;
    QGroupBox* m_outputGroup = nullptr;
    QGroupBox* m_infoGroup = nullptr;

    QLabel* m_algoLabel = nullptr;
    QComboBox* m_algoCombo = nullptr;
    QLabel* m_modeLabel = nullptr;
    QComboBox* m_modeCombo = nullptr;
    QLabel* m_inputLabel = nullptr;
    QPlainTextEdit* m_inputEdit = nullptr;
    QLineEdit* m_outputEdit = nullptr;
    QPushButton* m_copyBtn = nullptr;
    QLabel* m_statusLabel = nullptr;
    QLabel* m_formulaLabel = nullptr;
};

