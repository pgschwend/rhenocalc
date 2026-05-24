#include "unitconverterpage.h"
#include "ui/themecolors.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QFont>
#include <cmath>

UnitConverterPage::UnitConverterPage(QWidget* parent) : QWidget(parent) {
    // Define categories: name + units with factor to SI base
    // Frequency: base = Hz
    m_categoryNames = {
        "Frequency (Hz)", "Time / Period (s)", "Voltage (V)",
        "Current (A)", "Resistance (Ω)", "Capacitance (F)",
        "Data Rate (bps)", "Data Size (bytes)", "Temperature",
        "Power (W)", "Energy (J)", "Angle"
    };

    // Frequency
    m_categories.append({
        {"Hz",   1.0},
        {"kHz",  1e3},
        {"MHz",  1e6},
        {"GHz",  1e9},
        {"THz",  1e12},
        {"mHz",  1e-3},
    });
    // Time
    m_categories.append({
        {"s",    1.0},
        {"ms",   1e-3},
        {"µs",   1e-6},
        {"ns",   1e-9},
        {"ps",   1e-12},
        {"min",  60.0},
        {"h",    3600.0},
        {"day",  86400.0},
    });
    // Voltage
    m_categories.append({
        {"V",    1.0},
        {"mV",   1e-3},
        {"µV",   1e-6},
        {"nV",   1e-9},
        {"kV",   1e3},
        {"MV",   1e6},
    });
    // Current
    m_categories.append({
        {"A",    1.0},
        {"mA",   1e-3},
        {"µA",   1e-6},
        {"nA",   1e-9},
        {"kA",   1e3},
    });
    // Resistance
    m_categories.append({
        {"Ω",    1.0},
        {"mΩ",   1e-3},
        {"kΩ",   1e3},
        {"MΩ",   1e6},
        {"GΩ",   1e9},
    });
    // Capacitance
    m_categories.append({
        {"F",    1.0},
        {"mF",   1e-3},
        {"µF",   1e-6},
        {"nF",   1e-9},
        {"pF",   1e-12},
    });
    // Data Rate
    m_categories.append({
        {"bps",   1.0},
        {"kbps",  1e3},
        {"Mbps",  1e6},
        {"Gbps",  1e9},
        {"Byte/s",8.0},
        {"KB/s",  8e3},
        {"MB/s",  8e6},
        {"GB/s",  8e9},
    });
    // Data Size
    m_categories.append({
        {"Byte",   1.0},
        {"KB",     1024.0},
        {"MB",     1024.0*1024.0},
        {"GB",     1024.0*1024.0*1024.0},
        {"TB",     1024.0*1024.0*1024.0*1024.0},
        {"bit",    0.125},
        {"kbit",   125.0},
        {"Mbit",   125000.0},
    });
    // Temperature (special case - handled separately)
    m_categories.append({
        {"°C",  1.0},
        {"°F",  1.0},
        {"K",   1.0},
    });
    // Power
    m_categories.append({
        {"W",    1.0},
        {"mW",   1e-3},
        {"µW",   1e-6},
        {"kW",   1e3},
        {"MW",   1e6},
        {"dBm",  1.0}, // special
        {"hp",   745.7},
    });
    // Energy
    m_categories.append({
        {"J",    1.0},
        {"mJ",   1e-3},
        {"µJ",   1e-6},
        {"kJ",   1e3},
        {"MJ",   1e6},
        {"Wh",   3600.0},
        {"kWh",  3.6e6},
        {"eV",   1.60218e-19},
    });
    // Angle
    m_categories.append({
        {"°",    1.0},
        {"rad",  180.0 / M_PI},
        {"mrad", 0.18 / M_PI},
        {"grad", 0.9},
    });

    setupUI();
}

void UnitConverterPage::setupUI() {
    auto* root = new QVBoxLayout(this);
    root->setSizeConstraint(QLayout::SetNoConstraint);
    root->setSpacing(8);
    root->setContentsMargins(12, 12, 12, 12);

    auto* title = new QLabel("Unit Converter for Embedded Engineers", this);
    m_titleLabel = title;
    title->setStyleSheet("font-size:16px;font-weight:bold;");
    root->addWidget(title);

    // Category
    auto* catRow = new QHBoxLayout();
    m_catLabel = new QLabel("Category:", this);
    m_catLabel->setStyleSheet("font-size:13px;");
    m_catLabel->setFixedWidth(52);
    m_categoryCombo = new QComboBox(this);
    m_categoryCombo->addItems(m_categoryNames);
    m_categoryCombo->setMinimumWidth(120);
    catRow->addWidget(m_catLabel);
    catRow->addWidget(m_categoryCombo);
    catRow->addStretch();
    root->addLayout(catRow);

    // Converter box
    m_convGroup = new QGroupBox("Convert", this);
    m_convGroup->setStyleSheet(ThemeColors::unitGroupStyle(true));
    auto* convLayout = new QGridLayout(m_convGroup);
    convLayout->setSpacing(10);

    auto lbl = [&](const QString& t) {
        auto* l = new QLabel(t, this);
        l->setStyleSheet("font-size:13px;");
        return l;
    };

    convLayout->addWidget(lbl("From:"), 0, 0);
    m_fromEdit = new QLineEdit(this);
    m_fromEdit->setStyleSheet("font-family:'Consolas';font-size:18px;border-radius:4px;padding:6px 10px;");
    m_fromEdit->setPlaceholderText("Enter value...");
    convLayout->addWidget(m_fromEdit, 0, 1);

    m_fromUnit = new QComboBox(this);
    m_fromUnit->setStyleSheet("padding:4px 6px;border-radius:4px;font-size:12px;min-width:48px;");
    convLayout->addWidget(m_fromUnit, 0, 2);

    convLayout->addWidget(lbl("To:"), 1, 0);
    auto* toRow = new QHBoxLayout();

    m_resultLabel = new QLabel("—", this);
    m_resultLabel->setStyleSheet("font-family:'Consolas';font-size:16px;border-radius:4px;padding:4px 6px;min-width:96px;");
    m_resultLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    convLayout->addWidget(m_resultLabel, 1, 1);

    m_toUnit = new QComboBox(this);
    m_toUnit->setStyleSheet(m_fromUnit->styleSheet());
    convLayout->addWidget(m_toUnit, 1, 2);

    root->addWidget(m_convGroup);

    // Formula hint
    m_formulaLabel = new QLabel("", this);
    m_formulaLabel->setStyleSheet("font-size:12px;padding:4px;");
    m_formulaLabel->setWordWrap(true);
    root->addWidget(m_formulaLabel);

    // Quick reference table for the category
    m_refGroup = new QGroupBox("Quick Reference", this);
    m_refGroup->setStyleSheet(m_convGroup->styleSheet());
    auto* refLayout = new QGridLayout(m_refGroup);
    refLayout->setSpacing(6);
    // This will be populated on category change - we skip dynamic generation here
    // and just show useful embedded-specific hints via formulaLabel
    root->addWidget(m_refGroup);
    root->addStretch();

    // Connect
    connect(m_categoryCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &UnitConverterPage::onCategoryChanged);
    connect(m_fromEdit,  &QLineEdit::textChanged,     this, &UnitConverterPage::onFromValueChanged);
    connect(m_fromUnit,  QOverload<int>::of(&QComboBox::currentIndexChanged), this, &UnitConverterPage::onFromUnitChanged);
    connect(m_toUnit,    QOverload<int>::of(&QComboBox::currentIndexChanged), this, &UnitConverterPage::onToUnitChanged);

    onCategoryChanged(0);
}

void UnitConverterPage::populate(int category) {
    m_fromUnit->blockSignals(true);
    m_toUnit->blockSignals(true);
    m_fromUnit->clear();
    m_toUnit->clear();
    for (auto& u : m_categories[category]) {
        m_fromUnit->addItem(u.name);
        m_toUnit->addItem(u.name);
    }
    m_toUnit->setCurrentIndex(1); // default: second unit
    m_fromUnit->blockSignals(false);
    m_toUnit->blockSignals(false);
}

void UnitConverterPage::onCategoryChanged(int index) {
    populate(index);
    m_resultLabel->setText("—");
    m_fromEdit->clear();
    convert();
}

void UnitConverterPage::onFromValueChanged() { convert(); }
void UnitConverterPage::onFromUnitChanged()  { convert(); }
void UnitConverterPage::onToUnitChanged()    { convert(); }

void UnitConverterPage::convert() {
    bool ok;
    double fromVal = m_fromEdit->text().trimmed().toDouble(&ok);
    if (!ok || m_fromEdit->text().trimmed().isEmpty()) { m_resultLabel->setText("—"); return; }

    int cat     = m_categoryCombo->currentIndex();
    int fromIdx = m_fromUnit->currentIndex();
    int toIdx   = m_toUnit->currentIndex();
    if (fromIdx < 0 || toIdx < 0 || fromIdx >= m_categories[cat].size() || toIdx >= m_categories[cat].size()) return;

    // Temperature: special
    if (m_categoryNames[cat] == "Temperature") {
        double result = fromVal;
        QString fromU = m_fromUnit->currentText();
        QString toU   = m_toUnit->currentText();
        double c = fromVal;
        if      (fromU == "°F") c = (fromVal - 32.0) * 5.0 / 9.0;
        else if (fromU == "K")  c = fromVal - 273.15;
        if      (toU == "°F")  result = c * 9.0 / 5.0 + 32.0;
        else if (toU == "K")   result = c + 273.15;
        else                    result = c;
        m_resultLabel->setText(QString::number(result, 'g', 10) + " " + toU);
        m_formulaLabel->setText(QString("%1 %2 → %3 %4").arg(fromVal).arg(fromU).arg(result).arg(toU));
        return;
    }

    // Power: dBm special
    bool fromDbm = (m_fromUnit->currentText() == "dBm");
    bool toDbm   = (m_toUnit->currentText()   == "dBm");
    double baseVal;
    if (fromDbm) {
        baseVal = 1e-3 * std::pow(10.0, fromVal / 10.0); // dBm -> W
    } else {
        baseVal = fromVal * m_categories[cat][fromIdx].toBase;
    }
    double result;
    if (toDbm) {
        result = 10.0 * std::log10(baseVal / 1e-3); // W -> dBm
    } else {
        result = baseVal / m_categories[cat][toIdx].toBase;
    }

    // Format nicely
    QString resultStr;
    if (std::abs(result) >= 1e9 || (std::abs(result) < 1e-6 && result != 0))
        resultStr = QString::number(result, 'e', 6);
    else
        resultStr = QString::number(result, 'g', 10);

    m_resultLabel->setText(resultStr + " " + m_toUnit->currentText());
    m_formulaLabel->setText(
        QString("%1 %2 = %3 %4")
        .arg(fromVal).arg(m_fromUnit->currentText())
        .arg(resultStr).arg(m_toUnit->currentText())
    );
}

void UnitConverterPage::applyTheme(bool dark) {
    const QString grpS = ThemeColors::unitGroupStyle(dark);
    const QString fldS = ThemeColors::unitFieldStyle(dark);
    const QString resS = ThemeColors::unitResultStyle(dark);
    const QString ttlS = ThemeColors::unitTitleStyle(dark);
    const QString frmS = ThemeColors::unitFormulaStyle(dark);

    m_titleLabel->setStyleSheet(ttlS);
    m_formulaLabel->setStyleSheet(frmS);
    m_fromEdit->setStyleSheet(fldS);
    m_resultLabel->setStyleSheet(resS);
    m_convGroup->setStyleSheet(grpS);
    m_refGroup->setStyleSheet(grpS);
}
