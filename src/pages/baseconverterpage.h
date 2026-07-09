#pragma once

#include "ui/widgets/bitbutton.h"
#include <QLineEdit>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QGroupBox>
#include <vector>


class BaseConverterPage : public QWidget {
    Q_OBJECT
public:
    explicit BaseConverterPage(QWidget* parent = nullptr);
    void applyTheme(bool dark);

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

    QGroupBox* m_regGroup;
    QGroupBox* m_infoGroup;
    QLabel*    m_wLabel;
    QLabel*    m_fieldLabels[4];

    QLabel*    m_byteLabels[8];
    QLabel*    m_bitIndexLabels[32];
    std::vector<BitButton*> m_bitBtns;

    QLabel*    m_signedLabel;
    QLabel*    m_unsignedLabel;
    QLabel*    m_hexInfoLabel;
    QLabel*    m_floatLabel;

    int  m_wordBits = 32;
    bool m_updating = false;
    bool m_signed   = false;
    unsigned long long m_value = 0;
};
