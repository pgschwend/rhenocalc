#include "unitconverterpage.h"
#include "ui/themecolors.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QScrollArea>
#include <QSizePolicy>

UnitConverterPage::UnitConverterPage(QWidget* parent) : QWidget(parent) {
    m_categoryNames = Rheno::Core::defaultCategoryNames();
    m_categories = Rheno::Core::defaultCategories();

    setupUI();
}

void UnitConverterPage::setupUI() {
    setMinimumSize(0, 0);
    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->setSpacing(0);

    auto* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setFrameShape(QFrame::NoFrame);

    auto* content = new QWidget(scroll);
    content->setMinimumSize(0, 0);
    content->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);

    auto* root = new QVBoxLayout(content);
    root->setSizeConstraint(QLayout::SetNoConstraint);
    root->setSpacing(8);
    root->setContentsMargins(12, 12, 12, 12);

    auto* title = new QLabel("Unit Converter", this);
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
    m_convGroup->setStyleSheet(Rheno::UI::unitGroupStyle(true));
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
    root->addWidget(m_refGroup);
    root->addStretch();

    scroll->setWidget(content);
    outer->addWidget(scroll);

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
    updateQuickReference(index);
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

    const Rheno::Core::ConversionResult result = Rheno::Core::convert(
        fromVal, cat, fromIdx, toIdx, m_categories, m_categoryNames);
    if (!result.valid) {
        m_resultLabel->setText("—");
        m_formulaLabel->clear();
        return;
    }

    m_resultLabel->setText(result.resultText);
    m_formulaLabel->setText(result.formulaText);
}

void UnitConverterPage::applyTheme(bool dark) {
    const QString grpS = Rheno::UI::unitGroupStyle(dark);
    const QString fldS = Rheno::UI::unitFieldStyle(dark);
    const QString resS = Rheno::UI::unitResultStyle(dark);
    const QString ttlS = Rheno::UI::unitTitleStyle(dark);
    const QString frmS = Rheno::UI::unitFormulaStyle(dark);

    m_titleLabel->setStyleSheet(ttlS);
    m_formulaLabel->setStyleSheet(frmS);
    m_fromEdit->setStyleSheet(fldS);
    m_resultLabel->setStyleSheet(resS);
    m_convGroup->setStyleSheet(grpS);
    m_refGroup->setStyleSheet(grpS);
}

void UnitConverterPage::updateQuickReference(int category) {
    // Clear existing content
    QLayout* oldLayout = m_refGroup->layout();
    if (oldLayout) {
        QLayoutItem* item;
        while ((item = oldLayout->takeAt(0)) != nullptr) {
            delete item->widget();
            delete item;
        }
        delete oldLayout;
    }

    auto* layout = new QVBoxLayout(m_refGroup);
    layout->setSpacing(4);
    layout->setContentsMargins(8, 8, 8, 8);

    auto* refLabel = new QLabel(this);
    refLabel->setWordWrap(true);
    refLabel->setTextFormat(Qt::RichText);
    refLabel->setStyleSheet("font-size:12px; line-height:1.4;");

    QString ref;
    switch (category) {
        case 0: // Frequency (Hz)
            ref = R"(
<b>Frequency ↔ Period</b><br>
f = 1/T &nbsp;|&nbsp; T = 1/f<br><br>
<b>Prefixes:</b><br>
• 1 kHz = 1,000 Hz = 10³ Hz<br>
• 1 MHz = 1,000,000 Hz = 10⁶ Hz<br>
• 1 GHz = 1,000,000,000 Hz = 10⁹ Hz<br>
• 1 mHz = 0.001 Hz = 10⁻³ Hz<br><br>
<b>Period Units:</b><br>
• 1 ms = 0.001 s → f = 1 kHz<br>
• 1 µs = 0.000001 s → f = 1 MHz
)";
            break;

        case 1: // Time / Period (s)
            ref = R"(
<b>SI Prefixes:</b><br>
• 1 ms = 10⁻³ s (millisecond)<br>
• 1 µs = 10⁻⁶ s (microsecond)<br>
• 1 ns = 10⁻⁹ s (nanosecond)<br>
• 1 ps = 10⁻¹² s (picosecond)<br><br>
<b>Larger Units:</b><br>
• 1 min = 60 s<br>
• 1 h = 3,600 s (60 min)<br>
• 1 day = 86,400 s (24 h)
)";
            break;

        case 2: // Pressure
            ref = R"(
<b>Base Unit: Pascal (Pa)</b><br><br>
<b>Conversions:</b><br>
• 1 bar = 100,000 Pa = 100 kPa<br>
• 1 kPa = 1,000 Pa<br>
• 1 psi = 6,894.757 Pa<br><br>
<b>Common Values:</b><br>
• Atmospheric: ~1.013 bar ≈ 14.7 psi<br>
• 1 bar ≈ 14.5 psi
)";
            break;

        case 3: // Distance
            ref = R"(
<b>Base Unit: Meter (m)</b><br><br>
<b>Conversions:</b><br>
• 1 km = 1,000 m<br>
• 1 mile = 1,609.344 m ≈ 1.609 km<br>
• 1 nautical mile = 1,852 m<br><br>
<b>Quick Math:</b><br>
• km × 0.621 ≈ miles<br>
• miles × 1.609 ≈ km<br>
• 1 naut mile ≈ 1.151 miles
)";
            break;

        case 4: // Speed
            ref = R"(
<b>Base Unit: m/s</b><br><br>
<b>Conversions:</b><br>
• 1 m/s = 3.6 km/h<br>
• 1 km/h = 0.2778 m/s<br>
• 1 knot = 1.852 km/h = 0.5144 m/s<br>
• 1 mph = 1.609 km/h = 0.447 m/s<br><br>
<b>Quick Math:</b><br>
• km/h ÷ 1.852 = knots<br>
• mph × 1.609 = km/h
)";
            break;

        case 5: // Data Rate (bps)
            ref = R"(
<b>Bits vs Bytes:</b><br>
1 Byte = 8 bits<br><br>
<b>Bit Prefixes (decimal):</b><br>
• 1 kbps = 1,000 bps<br>
• 1 Mbps = 1,000,000 bps<br>
• 1 Gbps = 1,000,000,000 bps<br><br>
<b>Byte Rates:</b><br>
• 1 MB/s = 8 Mbps<br>
• 100 Mbps = 12.5 MB/s<br>
• 1 Gbps = 125 MB/s
)";
            break;

        case 6: // Data Size (bytes)
            ref = R"(
<b>Binary Prefixes (IEC):</b><br>
• 1 KB = 1,024 Bytes = 2¹⁰ B<br>
• 1 MB = 1,048,576 Bytes = 2²⁰ B<br>
• 1 GB = 1,073,741,824 Bytes = 2³⁰ B<br>
• 1 TB = 2⁴⁰ Bytes<br><br>
<b>Bits:</b><br>
• 1 Byte = 8 bits<br>
• 1 kbit = 1,000 bits = 125 Bytes<br>
• 1 Mbit = 1,000,000 bits = 125 KB
)";
            break;

        case 7: // Temperature
            ref = R"(
<b>Formulas:</b><br>
• °C → °F: °F = °C × 9/5 + 32<br>
• °F → °C: °C = (°F − 32) × 5/9<br>
• °C → K: K = °C + 273.15<br>
• K → °C: °C = K − 273.15<br><br>
<b>Reference Points:</b><br>
• 0°C = 32°F = 273.15 K (water freezes)<br>
• 100°C = 212°F = 373.15 K (water boils)<br>
• −40°C = −40°F
)";
            break;

        case 8: // Power (W)
            ref = R"(
<b>SI Prefixes:</b><br>
• 1 mW = 10⁻³ W<br>
• 1 µW = 10⁻⁶ W<br>
• 1 kW = 10³ W<br>
• 1 MW = 10⁶ W<br><br>
<b>dBm (Decibel-milliwatts):</b><br>
• P(dBm) = 10 × log₁₀(P / 1mW)<br>
• 0 dBm = 1 mW<br>
• 10 dBm = 10 mW<br>
• 30 dBm = 1 W<br><br>
<b>Horsepower:</b><br>
• 1 hp ≈ 745.7 W
)";
            break;

        case 9: // Energy (J)
            ref = R"(
<b>SI Prefixes:</b><br>
• 1 mJ = 10⁻³ J<br>
• 1 µJ = 10⁻⁶ J<br>
• 1 kJ = 10³ J<br>
• 1 MJ = 10⁶ J<br><br>
<b>Watt-hours:</b><br>
• 1 Wh = 3,600 J (P × t)<br>
• 1 kWh = 3,600,000 J = 3.6 MJ<br><br>
<b>Electron-volt:</b><br>
• 1 eV = 1.602 × 10⁻¹⁹ J<br>
<i>(energy of 1 electron through 1V)</i>
)";
            break;

        case 10: // Angle
            ref = R"(
<b>Conversions:</b><br>
• 1 full circle = 360° = 2π rad = 400 grad<br>
• 1 rad = 180°/π ≈ 57.2958°<br>
• 1° = π/180 rad ≈ 0.01745 rad<br>
• 1 grad = 0.9° (400 grad = 360°)<br><br>
<b>Common Values:</b><br>
• 90° = π/2 rad = 100 grad<br>
• 45° = π/4 rad = 50 grad<br>
• 1 mrad ≈ 0.0573°
)";
            break;

        default:
            ref = "No reference available.";
            break;
    }

    refLabel->setText(ref.trimmed());
    layout->addWidget(refLabel);
}

