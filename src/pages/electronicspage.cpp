#include "electronicspage.h"
#include "core/electronicscore.h"
#include "ui/themecolors.h"

#include <QComboBox>
#include <QDoubleValidator>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QRegularExpression>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QtMath>

ElectronicsPage::ElectronicsPage(QWidget* parent) : QWidget(parent) {
    setupUI();
    applyTheme(true);
    calcVoltageDivider();
    calcLedResistor();
    calcWheatstone();
    calcRCFilter();
    calcLCResonance();
    calcPullUpDown();
}

void ElectronicsPage::setupUI() {
    auto* root = new QVBoxLayout(this); // NOLINT(cppcoreguidelines-owning-memory)
    root->setSizeConstraint(QLayout::SetNoConstraint);
    root->setSpacing(0);
    root->setContentsMargins(0, 0, 0, 0);

    auto* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    auto* content = new QWidget(scroll);
    auto* contentLayout = new QVBoxLayout(content);
    contentLayout->setSizeConstraint(QLayout::SetNoConstraint);
    contentLayout->setSpacing(8);
    contentLayout->setContentsMargins(14, 12, 14, 12);

    m_titleLabel = new QLabel("Electronics Tools", this);
    contentLayout->addWidget(m_titleLabel);

    auto* validator = new QDoubleValidator(0.0, 1e15, 6, this); // NOLINT
    validator->setNotation(QDoubleValidator::StandardNotation);

    // ═══════════════════════════════════════════════════════════════════════════
    // Voltage Divider
    // ═══════════════════════════════════════════════════════════════════════════
    auto* vdGroup = new QGroupBox("Voltage Divider", this); // NOLINT
    auto* vdGrid = new QGridLayout(vdGroup); // NOLINT
    vdGrid->setHorizontalSpacing(10);
    vdGrid->setVerticalSpacing(6);

    int row = 0;
    vdGrid->addWidget(new QLabel("Vin (V)", this), row, 0);
    m_vdVin = new QLineEdit("5", this);
    m_vdVin->setValidator(validator);
    vdGrid->addWidget(m_vdVin, row, 1);
    row++;

    vdGrid->addWidget(new QLabel("R1 (Ω)", this), row, 0);
    m_vdR1 = new QLineEdit("10000", this);
    m_vdR1->setValidator(validator);
    vdGrid->addWidget(m_vdR1, row, 1);
    row++;

    vdGrid->addWidget(new QLabel("R2 (Ω)", this), row, 0);
    m_vdR2 = new QLineEdit("10000", this);
    m_vdR2->setValidator(validator);
    vdGrid->addWidget(m_vdR2, row, 1);
    row++;

    vdGrid->addWidget(new QLabel("Vout", this), row, 0);
    m_vdVout = new QLabel("—", this);
    m_vdVout->setTextInteractionFlags(Qt::TextSelectableByMouse);
    vdGrid->addWidget(m_vdVout, row, 1);
    row++;

    vdGrid->addWidget(new QLabel("Ratio", this), row, 0);
    m_vdRatio = new QLabel("—", this);
    m_vdRatio->setTextInteractionFlags(Qt::TextSelectableByMouse);
    vdGrid->addWidget(m_vdRatio, row, 1);

    vdGrid->setColumnStretch(1, 1);
    contentLayout->addWidget(vdGroup);

    // ═══════════════════════════════════════════════════════════════════════════
    // LED Resistor
    // ═══════════════════════════════════════════════════════════════════════════
    auto* ledGroup = new QGroupBox("LED Resistor", this); // NOLINT
    auto* ledGrid = new QGridLayout(ledGroup); // NOLINT
    ledGrid->setHorizontalSpacing(10);
    ledGrid->setVerticalSpacing(6);

    row = 0;
    ledGrid->addWidget(new QLabel("Vs (V)", this), row, 0);
    m_ledVs = new QLineEdit("5", this);
    m_ledVs->setValidator(validator);
    ledGrid->addWidget(m_ledVs, row, 1);
    row++;

    ledGrid->addWidget(new QLabel("Vf LED (V)", this), row, 0);
    m_ledVf = new QLineEdit("2.0", this);
    m_ledVf->setValidator(validator);
    ledGrid->addWidget(m_ledVf, row, 1);
    row++;

    ledGrid->addWidget(new QLabel("If LED (mA)", this), row, 0);
    m_ledIf = new QLineEdit("20", this);
    m_ledIf->setValidator(validator);
    ledGrid->addWidget(m_ledIf, row, 1);
    row++;

    ledGrid->addWidget(new QLabel("Resistor", this), row, 0);
    m_ledR = new QLabel("—", this);
    m_ledR->setTextInteractionFlags(Qt::TextSelectableByMouse);
    ledGrid->addWidget(m_ledR, row, 1);
    row++;

    ledGrid->addWidget(new QLabel("Power", this), row, 0);
    m_ledPower = new QLabel("—", this);
    m_ledPower->setTextInteractionFlags(Qt::TextSelectableByMouse);
    ledGrid->addWidget(m_ledPower, row, 1);

    ledGrid->setColumnStretch(1, 1);
    contentLayout->addWidget(ledGroup);

    // ═══════════════════════════════════════════════════════════════════════════
    // Wheatstone Bridge
    // ═══════════════════════════════════════════════════════════════════════════
    auto* wbGroup = new QGroupBox("Wheatstone Bridge", this); // NOLINT
    auto* wbGrid = new QGridLayout(wbGroup); // NOLINT
    wbGrid->setHorizontalSpacing(10);
    wbGrid->setVerticalSpacing(6);

    row = 0;
    wbGrid->addWidget(new QLabel("R1 (Ω)", this), row, 0);
    m_wbR1 = new QLineEdit("1000", this);
    m_wbR1->setValidator(validator);
    wbGrid->addWidget(m_wbR1, row, 1);
    row++;

    wbGrid->addWidget(new QLabel("R2 (Ω)", this), row, 0);
    m_wbR2 = new QLineEdit("1000", this);
    m_wbR2->setValidator(validator);
    wbGrid->addWidget(m_wbR2, row, 1);
    row++;

    wbGrid->addWidget(new QLabel("R3 (Ω)", this), row, 0);
    m_wbR3 = new QLineEdit("1000", this);
    m_wbR3->setValidator(validator);
    wbGrid->addWidget(m_wbR3, row, 1);
    row++;

    wbGrid->addWidget(new QLabel("Rx (calc)", this), row, 0);
    m_wbRx = new QLineEdit("", this);
    m_wbRx->setPlaceholderText("or enter to verify");
    m_wbRx->setValidator(validator);
    wbGrid->addWidget(m_wbRx, row, 1);
    row++;

    wbGrid->addWidget(new QLabel("Result", this), row, 0);
    m_wbResult = new QLabel("—", this);
    m_wbResult->setTextInteractionFlags(Qt::TextSelectableByMouse);
    wbGrid->addWidget(m_wbResult, row, 1);

    wbGrid->setColumnStretch(1, 1);
    contentLayout->addWidget(wbGroup);

    // ═══════════════════════════════════════════════════════════════════════════
    // RC Filter (Low-pass / High-pass)
    // ═══════════════════════════════════════════════════════════════════════════
    auto* rcGroup = new QGroupBox("RC Filter (Low/High-pass)", this); // NOLINT
    auto* rcGrid = new QGridLayout(rcGroup); // NOLINT
    rcGrid->setHorizontalSpacing(10);
    rcGrid->setVerticalSpacing(6);

    row = 0;
    rcGrid->addWidget(new QLabel("R (Ω)", this), row, 0);
    m_rcR = new QLineEdit("10000", this);
    m_rcR->setValidator(validator);
    rcGrid->addWidget(m_rcR, row, 1);
    row++;

    rcGrid->addWidget(new QLabel("C (nF)", this), row, 0);
    m_rcC = new QLineEdit("100", this);
    m_rcC->setValidator(validator);
    rcGrid->addWidget(m_rcC, row, 1);
    row++;

    rcGrid->addWidget(new QLabel("fc (cutoff)", this), row, 0);
    m_rcFc = new QLabel("—", this);
    m_rcFc->setTextInteractionFlags(Qt::TextSelectableByMouse);
    rcGrid->addWidget(m_rcFc, row, 1);
    row++;

    rcGrid->addWidget(new QLabel("τ (time const)", this), row, 0);
    m_rcTau = new QLabel("—", this);
    m_rcTau->setTextInteractionFlags(Qt::TextSelectableByMouse);
    rcGrid->addWidget(m_rcTau, row, 1);

    rcGrid->setColumnStretch(1, 1);
    contentLayout->addWidget(rcGroup);

    // ═══════════════════════════════════════════════════════════════════════════
    // LC Resonance
    // ═══════════════════════════════════════════════════════════════════════════
    auto* lcGroup = new QGroupBox("LC Resonance", this); // NOLINT
    auto* lcGrid = new QGridLayout(lcGroup); // NOLINT
    lcGrid->setHorizontalSpacing(10);
    lcGrid->setVerticalSpacing(6);

    row = 0;
    lcGrid->addWidget(new QLabel("L (µH)", this), row, 0);
    m_lcL = new QLineEdit("100", this);
    m_lcL->setValidator(validator);
    lcGrid->addWidget(m_lcL, row, 1);
    row++;

    lcGrid->addWidget(new QLabel("C (pF)", this), row, 0);
    m_lcC = new QLineEdit("100", this);
    m_lcC->setValidator(validator);
    lcGrid->addWidget(m_lcC, row, 1);
    row++;

    lcGrid->addWidget(new QLabel("f₀ (resonance)", this), row, 0);
    m_lcF0 = new QLabel("—", this);
    m_lcF0->setTextInteractionFlags(Qt::TextSelectableByMouse);
    lcGrid->addWidget(m_lcF0, row, 1);
    row++;

    lcGrid->addWidget(new QLabel("ω₀ (rad/s)", this), row, 0);
    m_lcOmega = new QLabel("—", this);
    m_lcOmega->setTextInteractionFlags(Qt::TextSelectableByMouse);
    lcGrid->addWidget(m_lcOmega, row, 1);

    lcGrid->setColumnStretch(1, 1);
    contentLayout->addWidget(lcGroup);

    // ═══════════════════════════════════════════════════════════════════════════
    // Pull-Up/Pull-Down Rise/Fall Time
    // ═══════════════════════════════════════════════════════════════════════════
    auto* puGroup = new QGroupBox("Pull-Up/Down Rise/Fall Time", this); // NOLINT
    auto* puGrid = new QGridLayout(puGroup); // NOLINT
    puGrid->setHorizontalSpacing(10);
    puGrid->setVerticalSpacing(6);

    row = 0;
    puGrid->addWidget(new QLabel("R (Ω)", this), row, 0);
    m_puR = new QLineEdit("10000", this);
    m_puR->setValidator(validator);
    puGrid->addWidget(m_puR, row, 1);
    row++;

    puGrid->addWidget(new QLabel("C (pF)", this), row, 0);
    m_puC = new QLineEdit("20", this);
    m_puC->setValidator(validator);
    puGrid->addWidget(m_puC, row, 1);
    row++;

    puGrid->addWidget(new QLabel("Vcc (V)", this), row, 0);
    m_puVcc = new QLineEdit("3.3", this);
    m_puVcc->setValidator(validator);
    puGrid->addWidget(m_puVcc, row, 1);
    row++;

    puGrid->addWidget(new QLabel("Vth (V)", this), row, 0);
    m_puVth = new QLineEdit("1.65", this);
    m_puVth->setValidator(validator);
    puGrid->addWidget(m_puVth, row, 1);
    row++;

    puGrid->addWidget(new QLabel("t_rise (0→Vth)", this), row, 0);
    m_puTrise = new QLabel("—", this);
    m_puTrise->setTextInteractionFlags(Qt::TextSelectableByMouse);
    puGrid->addWidget(m_puTrise, row, 1);
    row++;

    puGrid->addWidget(new QLabel("t_fall (Vcc→Vth)", this), row, 0);
    m_puTfall = new QLabel("—", this);
    m_puTfall->setTextInteractionFlags(Qt::TextSelectableByMouse);
    puGrid->addWidget(m_puTfall, row, 1);

    puGrid->setColumnStretch(1, 1);
    contentLayout->addWidget(puGroup);

    contentLayout->addStretch();
    scroll->setWidget(content);
    root->addWidget(scroll);

    // Connect signals
    auto connectAll = [this](QLineEdit* edit, auto slot) {
        connect(edit, &QLineEdit::textChanged, this, slot);
    };

    connectAll(m_vdVin, &ElectronicsPage::calcVoltageDivider);
    connectAll(m_vdR1, &ElectronicsPage::calcVoltageDivider);
    connectAll(m_vdR2, &ElectronicsPage::calcVoltageDivider);

    connectAll(m_ledVs, &ElectronicsPage::calcLedResistor);
    connectAll(m_ledVf, &ElectronicsPage::calcLedResistor);
    connectAll(m_ledIf, &ElectronicsPage::calcLedResistor);

    connectAll(m_wbR1, &ElectronicsPage::calcWheatstone);
    connectAll(m_wbR2, &ElectronicsPage::calcWheatstone);
    connectAll(m_wbR3, &ElectronicsPage::calcWheatstone);
    connectAll(m_wbRx, &ElectronicsPage::calcWheatstone);

    connectAll(m_rcR, &ElectronicsPage::calcRCFilter);
    connectAll(m_rcC, &ElectronicsPage::calcRCFilter);

    connectAll(m_lcL, &ElectronicsPage::calcLCResonance);
    connectAll(m_lcC, &ElectronicsPage::calcLCResonance);

    connectAll(m_puR, &ElectronicsPage::calcPullUpDown);
    connectAll(m_puC, &ElectronicsPage::calcPullUpDown);
    connectAll(m_puVcc, &ElectronicsPage::calcPullUpDown);
    connectAll(m_puVth, &ElectronicsPage::calcPullUpDown);
}

void ElectronicsPage::calcVoltageDivider() {
    double vin = 0, r1 = 0, r2 = 0;
    if (!Rheno::Core::parseValue(m_vdVin->text(), &vin) ||
        !Rheno::Core::parseValue(m_vdR1->text(), &r1) ||
        !Rheno::Core::parseValue(m_vdR2->text(), &r2)) {
        m_vdVout->setText("—");
        m_vdRatio->setText("—");
        return;
    }

    const auto result = Rheno::Core::calculateVoltageDivider(vin, r1, r2);
    if (!result.valid) {
        m_vdVout->setText("—");
        m_vdRatio->setText("—");
        return;
    }

    m_vdVout->setText(QString::number(result.vout, 'f', 4) + " V");
    m_vdRatio->setText(QString::number(result.ratio, 'f', 4));
}

void ElectronicsPage::calcLedResistor() {
    double vs = 0, vf = 0, ifMa = 0;
    if (!Rheno::Core::parseValue(m_ledVs->text(), &vs) ||
        !Rheno::Core::parseValue(m_ledVf->text(), &vf) ||
        !Rheno::Core::parseValue(m_ledIf->text(), &ifMa)) {
        m_ledR->setText("—");
        m_ledPower->setText("—");
        return;
    }

    const auto result = Rheno::Core::calculateLedResistor(vs, vf, ifMa);
    if (!result.valid) {
        m_ledR->setText("—");
        m_ledPower->setText("—");
        return;
    }

    m_ledR->setText(result.resistorText);
    m_ledPower->setText(result.powerText);
}

void ElectronicsPage::calcWheatstone() {
    double r1 = 0, r2 = 0, r3 = 0;
    if (!Rheno::Core::parseValue(m_wbR1->text(), &r1) ||
        !Rheno::Core::parseValue(m_wbR2->text(), &r2) ||
        !Rheno::Core::parseValue(m_wbR3->text(), &r3)) {
        m_wbResult->setText("—");
        return;
    }

    double rxEntered = 0;
    const bool hasRxEntered = Rheno::Core::parseValue(m_wbRx->text(), &rxEntered) && rxEntered > 0;
    const auto result = Rheno::Core::calculateWheatstone(r1, r2, r3, rxEntered, hasRxEntered);

    if (!result.valid) {
        m_wbResult->setText("—");
        return;
    }

    m_wbResult->setText(result.text);
}

void ElectronicsPage::calcRCFilter() {
    double r = 0, cNf = 0;
    if (!Rheno::Core::parseValue(m_rcR->text(), &r) ||
        !Rheno::Core::parseValue(m_rcC->text(), &cNf)) {
        m_rcFc->setText("—");
        m_rcTau->setText("—");
        return;
    }

    const auto result = Rheno::Core::calculateRcFilter(r, cNf);
    if (!result.valid) {
        m_rcFc->setText("—");
        m_rcTau->setText("—");
        return;
    }

    m_rcFc->setText(result.fcText);
    m_rcTau->setText(result.tauText);
}

void ElectronicsPage::calcLCResonance() {
    double lUh = 0, cPf = 0;
    if (!Rheno::Core::parseValue(m_lcL->text(), &lUh) ||
        !Rheno::Core::parseValue(m_lcC->text(), &cPf)) {
        m_lcF0->setText("—");
        m_lcOmega->setText("—");
        return;
    }

    const auto result = Rheno::Core::calculateLcResonance(lUh, cPf);
    if (!result.valid) {
        m_lcF0->setText("—");
        m_lcOmega->setText("—");
        return;
    }

    m_lcF0->setText(result.f0Text);
    m_lcOmega->setText(result.omegaText);
}

void ElectronicsPage::calcPullUpDown() {
    double r = 0, cPf = 0, vcc = 0, vth = 0;
    if (!Rheno::Core::parseValue(m_puR->text(), &r) ||
        !Rheno::Core::parseValue(m_puC->text(), &cPf) ||
        !Rheno::Core::parseValue(m_puVcc->text(), &vcc) ||
        !Rheno::Core::parseValue(m_puVth->text(), &vth)) {
        m_puTrise->setText("—");
        m_puTfall->setText("—");
        return;
    }

    const auto result = Rheno::Core::calculatePullUpDown(r, cPf, vcc, vth);
    if (!result.valid) {
        m_puTrise->setText("—");
        m_puTfall->setText("—");
        return;
    }

    m_puTrise->setText(result.riseText);
    m_puTfall->setText(result.fallText);
}

void ElectronicsPage::applyTheme(bool dark) {
    m_isDark = dark;

    const QString grpS = Rheno::UI::unitGroupStyle(dark);
    const QString fldS = Rheno::UI::unitFieldStyle(dark);
    const QString resS = Rheno::UI::unitResultStyle(dark);
    const QString ttlS = Rheno::UI::unitTitleStyle(dark);
    const QString frmS = Rheno::UI::unitFormulaStyle(dark);

    m_titleLabel->setStyleSheet(ttlS);

    for (auto* box : findChildren<QGroupBox*>())
        box->setStyleSheet(grpS);

    for (auto* edit : findChildren<QLineEdit*>())
        edit->setStyleSheet(fldS);

    // Style result labels
    QList<QLabel*> resultLabels = {
        m_vdVout, m_vdRatio,
        m_ledR, m_ledPower,
        m_wbResult,
        m_rcFc, m_rcTau,
        m_lcF0, m_lcOmega,
        m_puTrise, m_puTfall
    };
    for (auto* label : resultLabels)
        label->setStyleSheet(resS);

    for (auto* label : findChildren<QLabel*>())
        if (label != m_titleLabel && !resultLabels.contains(label))
            label->setStyleSheet(frmS);

    m_titleLabel->setStyleSheet(ttlS);
}

