#include "financepage.h"
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
#include <QtMath>

namespace {

double clampNonNegative(double value) {
    return value < 0.0 ? 0.0 : value;
}

int compoundingPerYear(const QString& label) {
    if (label == "Daily") return 365;
    if (label == "Monthly") return 12;
    if (label == "Quarterly") return 4;
    return 1;
}

int contributionsPerYear(const QString& label) {
    if (label == "None") return 0;
    if (label == "Monthly") return 12;
    if (label == "Quarterly") return 4;
    return 1;
}

} // namespace

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

bool FinancePage::parseDouble(const QString& text, double* value) const {
    QString trimmed = text.trimmed();
    if (trimmed.isEmpty()) {
        *value = 0.0;
        return true;
    }

    bool ok = false;
    double parsed = QLocale::system().toDouble(trimmed, &ok);
    if (!ok) {
        QString normalized = trimmed;
        parsed = normalized.replace(',', '.').toDouble(&ok);
    }
    if (!ok) return false;
    *value = parsed;
    return true;
}

void FinancePage::setMoneyLabel(QLabel* label, double value) const {
    const QLocale loc = QLocale::system();
    label->setText(loc.toString(value, 'f', 2));
}

void FinancePage::recalc() {
    double simplePrincipal = 0.0;
    double simpleRate = 0.0;
    double simplePeriods = 0.0;

    if (!parseDouble(m_simplePrincipalEdit->text(), &simplePrincipal)) return;
    if (!parseDouble(m_simpleRateEdit->text(), &simpleRate)) return;
    if (!parseDouble(m_simplePeriodsEdit->text(), &simplePeriods)) return;

    simplePrincipal = clampNonNegative(simplePrincipal);
    simpleRate = clampNonNegative(simpleRate);
    simplePeriods = clampNonNegative(simplePeriods);
    const double simpleResult = simplePrincipal * qPow(1.0 + (simpleRate / 100.0), simplePeriods);
    setMoneyLabel(m_simpleResultLabel, simpleResult);

    double principal = 0.0;
    double contrib = 0.0;
    double annualRate = 0.0;
    double years = 0.0;

    if (!parseDouble(m_principalEdit->text(), &principal)) return;
    if (!parseDouble(m_contribEdit->text(), &contrib)) return;
    if (!parseDouble(m_rateEdit->text(), &annualRate)) return;
    if (!parseDouble(m_yearsEdit->text(), &years)) return;

    principal = clampNonNegative(principal);
    contrib = clampNonNegative(contrib);
    annualRate = clampNonNegative(annualRate);
    years = clampNonNegative(years);

    const int n = compoundingPerYear(m_compoundCombo->currentText());
    const int m = contributionsPerYear(m_contribFreqCombo->currentText());

    const double periodRate = (annualRate / 100.0) / static_cast<double>(n);
    const int totalPeriods = static_cast<int>(qRound(years * n));

    double balance = principal;
    double totalContrib = principal;

    const bool contribAtStart = m_contribBeginCheck->isChecked();
    const double contribInterval = (m > 0) ? (1.0 / static_cast<double>(m)) : 0.0;
    double nextContribution = (m > 0) ? (contribAtStart ? 0.0 : contribInterval) : 1e9;
    const double epsilon = 1e-9;

    for (int i = 1; i <= totalPeriods; ++i) {
        const double periodStart = (static_cast<double>(i - 1)) / static_cast<double>(n);
        const double periodEnd = (static_cast<double>(i)) / static_cast<double>(n);

        if (m > 0 && contribAtStart) {
            while (nextContribution <= periodStart + epsilon) {
                balance += contrib;
                totalContrib += contrib;
                nextContribution += contribInterval;
            }
        }

        balance *= (1.0 + periodRate);

        if (m > 0 && !contribAtStart) {
            while (nextContribution <= periodEnd + epsilon) {
                balance += contrib;
                totalContrib += contrib;
                nextContribution += contribInterval;
            }
        }
    }

    const double totalInterest = balance - totalContrib;
    const double effectiveRate = qPow(1.0 + periodRate, n) - 1.0;

    setMoneyLabel(m_futureValueLabel, balance);
    setMoneyLabel(m_totalContribLabel, totalContrib);
    setMoneyLabel(m_totalInterestLabel, totalInterest);
    m_effectiveRateLabel->setText(QLocale::system().toString(effectiveRate * 100.0, 'f', 3) + "%");
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
