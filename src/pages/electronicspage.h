#pragma once

#include <QWidget>

class QLabel;
class QLineEdit;
class QComboBox;
class QGroupBox;

class ElectronicsPage : public QWidget {
    Q_OBJECT

public:
    explicit ElectronicsPage(QWidget* parent = nullptr);
    void applyTheme(bool dark);

private:
    void setupUI();
    void calcVoltageDivider();
    void calcLedResistor();
    void calcWheatstone();
    void calcRCFilter();
    void calcLCResonance();
    void calcPullUpDown();

    QLabel* m_titleLabel = nullptr;

    // Voltage Divider
    QLineEdit* m_vdVin = nullptr;
    QLineEdit* m_vdR1 = nullptr;
    QLineEdit* m_vdR2 = nullptr;
    QLabel* m_vdVout = nullptr;
    QLabel* m_vdRatio = nullptr;

    // LED Resistor
    QLineEdit* m_ledVs = nullptr;
    QLineEdit* m_ledVf = nullptr;
    QLineEdit* m_ledIf = nullptr;
    QLabel* m_ledR = nullptr;
    QLabel* m_ledPower = nullptr;

    // Wheatstone Bridge
    QLineEdit* m_wbR1 = nullptr;
    QLineEdit* m_wbR2 = nullptr;
    QLineEdit* m_wbR3 = nullptr;
    QLineEdit* m_wbRx = nullptr;
    QLabel* m_wbResult = nullptr;

    // RC Filter
    QLineEdit* m_rcR = nullptr;
    QLineEdit* m_rcC = nullptr;
    QLabel* m_rcFc = nullptr;
    QLabel* m_rcTau = nullptr;

    // LC Resonance
    QLineEdit* m_lcL = nullptr;
    QLineEdit* m_lcC = nullptr;
    QLabel* m_lcF0 = nullptr;
    QLabel* m_lcOmega = nullptr;

    // Pull-Up/Down Rise/Fall Time
    QLineEdit* m_puR = nullptr;
    QLineEdit* m_puC = nullptr;
    QLineEdit* m_puVcc = nullptr;
    QLineEdit* m_puVth = nullptr;
    QLabel* m_puTrise = nullptr;
    QLabel* m_puTfall = nullptr;

    bool m_isDark = true;
};

