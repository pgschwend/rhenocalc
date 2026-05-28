#include "unitconverterpage.h"
#include "ui/themecolors.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>

UnitConverterPage::UnitConverterPage(QWidget* parent) : QWidget(parent) {
    m_categoryNames = UnitConverterCore::defaultCategoryNames();
    m_categories = UnitConverterCore::defaultCategories();

    setupUI();
}

void UnitConverterPage::setupUI() {
    auto* root = new QVBoxLayout(this);
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
    new QGridLayout(m_refGroup);
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

    const UnitConverterCore::ConversionResult result = UnitConverterCore::convert(
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
