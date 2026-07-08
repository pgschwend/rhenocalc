#include "financepage.h"
#include "core/finance.h"
#include "ui/themecolors.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDoubleValidator>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QLocale>
#include <QScrollArea>
#include <QVBoxLayout>

FinancePage::FinancePage(QWidget* parent) : QWidget(parent) {
    setupUI();
    applyTheme(true);
    recalc();
}

void FinancePage::setupUI() {
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

    m_titleLabel = new QLabel("Finance Tools", this);
    contentLayout->addWidget(m_titleLabel);

    auto* moneyValidator = new QDoubleValidator(0.0, 1e12, 2, this); // NOLINT(cppcoreguidelines-owning-memory)
    moneyValidator->setNotation(QDoubleValidator::StandardNotation);
    moneyValidator->setLocale(QLocale::system());

    auto* rateValidator = new QDoubleValidator(0.0, 1000.0, 4, this); // NOLINT(cppcoreguidelines-owning-memory)
    rateValidator->setNotation(QDoubleValidator::StandardNotation);
    rateValidator->setLocale(QLocale::system());

    auto* periodValidator = new QDoubleValidator(0.0, 100000.0, 2, this); // NOLINT(cppcoreguidelines-owning-memory)
    periodValidator->setNotation(QDoubleValidator::StandardNotation);
    periodValidator->setLocale(QLocale::system());

    auto setFieldMinHeight = [](QWidget* w, int h) {
        w->setMinimumHeight(h);
    };

    auto* simpleGroup = new QGroupBox("Simple Compound", this); // NOLINT(cppcoreguidelines-owning-memory)
    auto* simpleGrid = new QGridLayout(simpleGroup); // NOLINT(cppcoreguidelines-owning-memory)
    simpleGrid->setHorizontalSpacing(10);
    simpleGrid->setVerticalSpacing(6);

    int simpleRow = 0;
    simpleGrid->addWidget(new QLabel("Start amount", this), simpleRow, 0);
    m_simplePrincipalEdit = new QLineEdit("1000", this);
    m_simplePrincipalEdit->setValidator(moneyValidator);
    const int fieldHeight = qMax(m_simplePrincipalEdit->sizeHint().height(), 26) + 8;
    setFieldMinHeight(m_simplePrincipalEdit, fieldHeight);
    simpleGrid->addWidget(m_simplePrincipalEdit, simpleRow, 1);
    simpleRow++;

    simpleGrid->addWidget(new QLabel("Rate (%)", this), simpleRow, 0);
    m_simpleRateEdit = new QLineEdit("5", this);
    m_simpleRateEdit->setValidator(rateValidator);
    setFieldMinHeight(m_simpleRateEdit, fieldHeight);
    simpleGrid->addWidget(m_simpleRateEdit, simpleRow, 1);
    simpleRow++;

    simpleGrid->addWidget(new QLabel("Periods", this), simpleRow, 0);
    m_simplePeriodsEdit = new QLineEdit("10", this);
    m_simplePeriodsEdit->setValidator(periodValidator);
    setFieldMinHeight(m_simplePeriodsEdit, fieldHeight);
    simpleGrid->addWidget(m_simplePeriodsEdit, simpleRow, 1);
    simpleRow++;

    auto* simpleResultTitle = new QLabel("Result", this);
    simpleGrid->addWidget(simpleResultTitle, simpleRow, 0);
    m_simpleResultLabel = new QLabel("—", this);
    simpleGrid->addWidget(m_simpleResultLabel, simpleRow, 1);

    simpleGrid->setColumnStretch(1, 1);
    contentLayout->addWidget(simpleGroup);

    auto* compoundGroup = new QGroupBox("Compound Interest", this); // NOLINT(cppcoreguidelines-owning-memory)
    auto* compoundLayout = new QVBoxLayout(compoundGroup); // NOLINT(cppcoreguidelines-owning-memory)
    auto* compoundGrid = new QGridLayout();
    compoundGrid->setHorizontalSpacing(10);
    compoundGrid->setVerticalSpacing(6);

    int row = 0;
    compoundGrid->addWidget(new QLabel("Initial investment", this), row, 0);
    m_principalEdit = new QLineEdit("1000", this);
    m_principalEdit->setValidator(moneyValidator);
    setFieldMinHeight(m_principalEdit, fieldHeight);
    compoundGrid->addWidget(m_principalEdit, row, 1);
    row++;

    compoundGrid->addWidget(new QLabel("Annual rate (%)", this), row, 0);
    m_rateEdit = new QLineEdit("5", this);
    m_rateEdit->setValidator(rateValidator);
    setFieldMinHeight(m_rateEdit, fieldHeight);
    compoundGrid->addWidget(m_rateEdit, row, 1);
    row++;

    compoundGrid->addWidget(new QLabel("Contribution", this), row, 0);
    m_contribEdit = new QLineEdit("100", this);
    m_contribEdit->setValidator(moneyValidator);
    setFieldMinHeight(m_contribEdit, fieldHeight);
    compoundGrid->addWidget(m_contribEdit, row, 1);
    row++;

    compoundGrid->addWidget(new QLabel("Contribution frequency", this), row, 0);
    m_contribFreqCombo = new QComboBox(this);
    m_contribFreqCombo->addItems({"None", "Monthly", "Quarterly", "Yearly"});
    m_contribFreqCombo->setCurrentIndex(1);
    setFieldMinHeight(m_contribFreqCombo, fieldHeight);
    compoundGrid->addWidget(m_contribFreqCombo, row, 1);
    row++;

    compoundGrid->addWidget(new QLabel("Years", this), row, 0);
    m_yearsEdit = new QLineEdit("10", this);
    m_yearsEdit->setValidator(periodValidator);
    setFieldMinHeight(m_yearsEdit, fieldHeight);
    compoundGrid->addWidget(m_yearsEdit, row, 1);
    row++;

    compoundGrid->addWidget(new QLabel("Compounding", this), row, 0);
    m_compoundCombo = new QComboBox(this);
    m_compoundCombo->addItems({"Monthly", "Quarterly", "Yearly", "Daily"});
    m_compoundCombo->setCurrentIndex(0);
    setFieldMinHeight(m_compoundCombo, fieldHeight);
    compoundGrid->addWidget(m_compoundCombo, row, 1);
    row++;

    m_contribBeginCheck = new QCheckBox("Contributions at period start", this);
    compoundGrid->addWidget(m_contribBeginCheck, row, 0, 1, 2);

    compoundGrid->setColumnStretch(1, 1);
    compoundLayout->addLayout(compoundGrid);
    compoundLayout->addSpacing(6);

    auto* resultTitle = new QLabel("Result", this);
    resultTitle->setContentsMargins(10, 4, 0, 0);
    m_resultTitleLabel = resultTitle;
    compoundLayout->addWidget(resultTitle);

    auto* resultGrid = new QGridLayout();
    resultGrid->setHorizontalSpacing(10);
    resultGrid->setVerticalSpacing(6);

    resultGrid->addWidget(new QLabel("Future value", this), 0, 0);
    m_futureValueLabel = new QLabel("—", this);
    resultGrid->addWidget(m_futureValueLabel, 0, 1);

    resultGrid->addWidget(new QLabel("Total contributions", this), 1, 0);
    m_totalContribLabel = new QLabel("—", this);
    resultGrid->addWidget(m_totalContribLabel, 1, 1);

    resultGrid->addWidget(new QLabel("Total interest", this), 2, 0);
    m_totalInterestLabel = new QLabel("—", this);
    resultGrid->addWidget(m_totalInterestLabel, 2, 1);

    resultGrid->addWidget(new QLabel("Effective annual rate", this), 3, 0);
    m_effectiveRateLabel = new QLabel("—", this);
    resultGrid->addWidget(m_effectiveRateLabel, 3, 1);

    resultGrid->setColumnStretch(1, 1);
    compoundLayout->addLayout(resultGrid);
    contentLayout->addWidget(compoundGroup);

    contentLayout->addStretch();
    scroll->setWidget(content);
    root->addWidget(scroll);

    connect(m_simplePrincipalEdit, &QLineEdit::textChanged, this, [this]() { recalc(); });
    connect(m_simpleRateEdit, &QLineEdit::textChanged, this, [this]() { recalc(); });
    connect(m_simplePeriodsEdit, &QLineEdit::textChanged, this, [this]() { recalc(); });

    connect(m_principalEdit, &QLineEdit::textChanged, this, [this]() { recalc(); });
    connect(m_contribEdit, &QLineEdit::textChanged, this, [this]() { recalc(); });
    connect(m_rateEdit, &QLineEdit::textChanged, this, [this]() { recalc(); });
    connect(m_yearsEdit, &QLineEdit::textChanged, this, [this]() { recalc(); });
    connect(m_compoundCombo, &QComboBox::currentTextChanged, this, [this]() { recalc(); });
    connect(m_contribFreqCombo, &QComboBox::currentTextChanged, this, [this]() { recalc(); });
    connect(m_contribBeginCheck, &QCheckBox::toggled, this, [this]() { recalc(); });
}

void FinancePage::recalc() {
    double simplePrincipal = 0.0;
    double simpleRate = 0.0;
    double simplePeriods = 0.0;

    if (!Rheno::Core::parseDouble(m_simplePrincipalEdit->text(), &simplePrincipal)) return;
    if (!Rheno::Core::parseDouble(m_simpleRateEdit->text(), &simpleRate)) return;
    if (!Rheno::Core::parseDouble(m_simplePeriodsEdit->text(), &simplePeriods)) return;

    const auto simple = Rheno::Core::calculateSimpleCompound(simplePrincipal, simpleRate, simplePeriods);
    m_simpleResultLabel->setText(Rheno::Core::formatMoney(simple.futureValue));

    double principal = 0.0;
    double contrib = 0.0;
    double annualRate = 0.0;
    double years = 0.0;

    if (!Rheno::Core::parseDouble(m_principalEdit->text(), &principal)) return;
    if (!Rheno::Core::parseDouble(m_contribEdit->text(), &contrib)) return;
    if (!Rheno::Core::parseDouble(m_rateEdit->text(), &annualRate)) return;
    if (!Rheno::Core::parseDouble(m_yearsEdit->text(), &years)) return;

    const auto compound = Rheno::Core::calculateCompoundInterest(
        principal,
        annualRate,
        contrib,
        years,
        Rheno::Core::compoundingPerYear(m_compoundCombo->currentText()),
        Rheno::Core::contributionsPerYear(m_contribFreqCombo->currentText()),
        m_contribBeginCheck->isChecked());

    m_futureValueLabel->setText(Rheno::Core::formatMoney(compound.futureValue));
    m_totalContribLabel->setText(Rheno::Core::formatMoney(compound.totalContributions));
    m_totalInterestLabel->setText(Rheno::Core::formatMoney(compound.totalInterest));
    m_effectiveRateLabel->setText(QLocale::system().toString(compound.effectiveAnnualRate * 100.0, 'f', 3) + "%");
}

void FinancePage::applyTheme(bool dark) {
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

    m_futureValueLabel->setStyleSheet(resS);
    m_totalContribLabel->setStyleSheet(resS);
    m_totalInterestLabel->setStyleSheet(resS);
    m_effectiveRateLabel->setStyleSheet(resS);
    m_simpleResultLabel->setStyleSheet(resS);

    for (auto* label : findChildren<QLabel*>())
        label->setStyleSheet(frmS);

    if (m_simpleResultLabel)
        m_simpleResultLabel->setStyleSheet(resS + "font-size:16px;");

    if (m_futureValueLabel)
        m_futureValueLabel->setStyleSheet(resS + "font-size:13px;");

    if (m_totalContribLabel)
        m_totalContribLabel->setStyleSheet(resS + "font-size:13px;");

    if (m_totalInterestLabel)
        m_totalInterestLabel->setStyleSheet(resS + "font-size:13px;");

    if (m_effectiveRateLabel)
        m_effectiveRateLabel->setStyleSheet(resS + "font-size:13px;");

    if (m_resultTitleLabel)
        m_resultTitleLabel->setStyleSheet(Rheno::UI::unitTitleStyle(dark) + "font-size:13px;");

    m_titleLabel->setStyleSheet(ttlS);
}
