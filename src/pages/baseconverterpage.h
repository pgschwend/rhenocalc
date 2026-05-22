#pragma once
#include <QWidget>
#include <QLineEdit>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QCheckBox>
#include <vector>

class BitButton : public QPushButton {
    Q_OBJECT
public:
    explicit BitButton(int bitIndex, QWidget* parent = nullptr);
    void setState(bool on);
    bool state() const { return m_state; }
    int  bitIndex() const { return m_bitIndex; }
signals:
    void toggled2(int bit, bool state);
private:
    void refresh();
    int  m_bitIndex;
    bool m_state = false;
};

class BaseConverterPage : public QWidget {
    Q_OBJECT
public:
    explicit BaseConverterPage(QWidget* parent = nullptr);

private slots:
    void onHexChanged();
    void onDecChanged();
    void onBinChanged();
    void onOctChanged();
    void onBitToggled(int bit, bool state);
    void onWordWidthChanged(int index);
    void onSignedToggled(bool checked);

private:
    void setupUI();
    void updateAll(unsigned long long value, QLineEdit* skip = nullptr);
    void updateBitButtons(unsigned long long value);
    void updateInfoLabels(unsigned long long value);

    QLineEdit* m_hexEdit;
    QLineEdit* m_decEdit;
    QLineEdit* m_binEdit;
    QLineEdit* m_octEdit;
    QComboBox* m_widthCombo;
    QCheckBox* m_signedCheck;

    QLabel*    m_byteLabels[8];   // byte values
    QLabel*    m_bitIndexLabels[32]; // bit index labels (top)
    std::vector<BitButton*> m_bitBtns; // 64 max

    QLabel*    m_signedLabel;
    QLabel*    m_unsignedLabel;
    QLabel*    m_hexInfoLabel;
    QLabel*    m_floatLabel;

    int  m_wordBits = 32;
    bool m_updating = false;
    bool m_signed   = false;
    unsigned long long m_value = 0;
};

