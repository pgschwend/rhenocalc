#include "floatpage.h"
#include "core/floatcore.h"
#include "ui/themecolors.h"

#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QScrollArea>
#include <QVBoxLayout>

FloatPage::FloatPage(QWidget* parent) : QWidget(parent) {
    setupUI();
    applyTheme(true);
    floatToBinary();
    binaryToFloat();
}

void FloatPage::setupUI() {
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

    m_titleLabel = new QLabel("IEEE 754 Float Converter", this);
    contentLayout->addWidget(m_titleLabel);

    // ═══════════════════════════════════════════════════════════════════════════
    // Float to Binary Section
    // ═══════════════════════════════════════════════════════════════════════════
    auto* floatGroup = new QGroupBox("Float → Binary", this); // NOLINT(cppcoreguidelines-owning-memory)
    auto* floatGrid = new QGridLayout(floatGroup); // NOLINT(cppcoreguidelines-owning-memory)
    floatGrid->setHorizontalSpacing(10);
    floatGrid->setVerticalSpacing(6);

    int row = 0;
    floatGrid->addWidget(new QLabel("Type", this), row, 0);
    m_floatTypeCombo = new QComboBox(this);
    m_floatTypeCombo->addItems({"float16 (Half)", "float32 (Single)", "float64 (Double)"});
    m_floatTypeCombo->setCurrentIndex(1); // Default float32
    floatGrid->addWidget(m_floatTypeCombo, row, 1);
    row++;

    floatGrid->addWidget(new QLabel("Decimal", this), row, 0);
    m_floatInput = new QLineEdit("3.14159", this);
    floatGrid->addWidget(m_floatInput, row, 1);
    row++;

    floatGrid->addWidget(new QLabel("Sign", this), row, 0);
    m_signLabel = new QLabel("—", this);
    m_signLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    floatGrid->addWidget(m_signLabel, row, 1);
    row++;

    floatGrid->addWidget(new QLabel("Exponent", this), row, 0);
    m_exponentLabel = new QLabel("—", this);
    m_exponentLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    floatGrid->addWidget(m_exponentLabel, row, 1);
    row++;

    floatGrid->addWidget(new QLabel("Mantissa", this), row, 0);
    m_mantissaLabel = new QLabel("—", this);
    m_mantissaLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    floatGrid->addWidget(m_mantissaLabel, row, 1);
    row++;

    floatGrid->addWidget(new QLabel("Binary", this), row, 0);
    m_binaryLabel = new QLabel("—", this);
    m_binaryLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_binaryLabel->setWordWrap(true);
    floatGrid->addWidget(m_binaryLabel, row, 1);
    row++;

    floatGrid->addWidget(new QLabel("Hex", this), row, 0);
    m_hexLabel = new QLabel("—", this);
    m_hexLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    floatGrid->addWidget(m_hexLabel, row, 1);

    floatGrid->setColumnStretch(1, 1);
    contentLayout->addWidget(floatGroup);

    // ═══════════════════════════════════════════════════════════════════════════
    // Binary to Float Section
    // ═══════════════════════════════════════════════════════════════════════════
    auto* binGroup = new QGroupBox("Binary → Float", this); // NOLINT(cppcoreguidelines-owning-memory)
    auto* binGrid = new QGridLayout(binGroup); // NOLINT(cppcoreguidelines-owning-memory)
    binGrid->setHorizontalSpacing(10);
    binGrid->setVerticalSpacing(6);

    row = 0;
    binGrid->addWidget(new QLabel("Type", this), row, 0);
    m_binTypeCombo = new QComboBox(this);
    m_binTypeCombo->addItems({"float16 (Half)", "float32 (Single)", "float64 (Double)"});
    m_binTypeCombo->setCurrentIndex(1);
    binGrid->addWidget(m_binTypeCombo, row, 1);
    row++;

    binGrid->addWidget(new QLabel("Binary", this), row, 0);
    m_binaryInput = new QLineEdit("01000000010010010000111111011011", this); // ~3.14159 in float32
    binGrid->addWidget(m_binaryInput, row, 1);
    row++;

    binGrid->addWidget(new QLabel("Sign", this), row, 0);
    m_binSignLabel = new QLabel("—", this);
    m_binSignLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    binGrid->addWidget(m_binSignLabel, row, 1);
    row++;

    binGrid->addWidget(new QLabel("Exponent", this), row, 0);
    m_binExponentLabel = new QLabel("—", this);
    m_binExponentLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    binGrid->addWidget(m_binExponentLabel, row, 1);
    row++;

    binGrid->addWidget(new QLabel("Mantissa", this), row, 0);
    m_binMantissaLabel = new QLabel("—", this);
    m_binMantissaLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    binGrid->addWidget(m_binMantissaLabel, row, 1);
    row++;

    binGrid->addWidget(new QLabel("Decimal", this), row, 0);
    m_floatResultLabel = new QLabel("—", this);
    m_floatResultLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    binGrid->addWidget(m_floatResultLabel, row, 1);
    row++;

    binGrid->addWidget(new QLabel("Hex", this), row, 0);
    m_hexResultLabel = new QLabel("—", this);
    m_hexResultLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    binGrid->addWidget(m_hexResultLabel, row, 1);

    binGrid->setColumnStretch(1, 1);
    contentLayout->addWidget(binGroup);

    contentLayout->addStretch();
    scroll->setWidget(content);
    root->addWidget(scroll);

    // Connect signals
    connect(m_floatInput, &QLineEdit::textChanged, this, &FloatPage::floatToBinary);
    connect(m_floatTypeCombo, &QComboBox::currentTextChanged, this, &FloatPage::floatToBinary);
    connect(m_binaryInput, &QLineEdit::textChanged, this, &FloatPage::binaryToFloat);
    connect(m_binTypeCombo, &QComboBox::currentTextChanged, this, &FloatPage::binaryToFloat);
}

void FloatPage::floatToBinary() {
    const auto result = Rheno::Core::encodeFloatText(m_floatInput->text(), m_floatTypeCombo->currentText());
    if (!result.valid) {
        if (result.error == "empty") {
            m_signLabel->setText("—");
        } else {
            m_signLabel->setText("Invalid");
        }
        m_exponentLabel->setText("—");
        m_mantissaLabel->setText("—");
        m_binaryLabel->setText("—");
        m_hexLabel->setText("—");
        return;
    }

    m_signLabel->setText(result.sign + (result.sign == "0" ? " (+)" : " (−)"));
    m_exponentLabel->setText(result.exponent + " (" + QString::number(result.exponent.toInt(nullptr, 2)) + ")");
    m_mantissaLabel->setText(result.mantissa);
    m_binaryLabel->setText(result.binarySpaced);
    m_hexLabel->setText(result.hex);
}

void FloatPage::binaryToFloat() {
    const auto result = Rheno::Core::decodeFloatBits(m_binaryInput->text(), m_binTypeCombo->currentText());
    if (!result.valid) {
        m_binSignLabel->setText(result.error.isEmpty() ? "—" : result.error);
        m_binExponentLabel->setText("—");
        m_binMantissaLabel->setText("—");
        m_floatResultLabel->setText("—");
        m_hexResultLabel->setText("—");
        return;
    }

    m_binSignLabel->setText(result.sign + (result.sign == "0" ? " (+)" : " (−)"));
    m_binExponentLabel->setText(result.exponent + " (" + QString::number(result.exponent.toInt(nullptr, 2)) + ")");
    m_binMantissaLabel->setText(result.mantissa);
    m_floatResultLabel->setText(result.decimal);
    m_hexResultLabel->setText(result.hex);
}

void FloatPage::applyTheme(bool dark) {
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
    m_signLabel->setStyleSheet(resS);
    m_exponentLabel->setStyleSheet(resS);
    m_mantissaLabel->setStyleSheet(resS);
    m_binaryLabel->setStyleSheet(resS);
    m_hexLabel->setStyleSheet(resS);

    m_binSignLabel->setStyleSheet(resS);
    m_binExponentLabel->setStyleSheet(resS);
    m_binMantissaLabel->setStyleSheet(resS);
    m_floatResultLabel->setStyleSheet(resS);
    m_hexResultLabel->setStyleSheet(resS);

    for (auto* label : findChildren<QLabel*>())
        if (label != m_titleLabel &&
            label != m_signLabel && label != m_exponentLabel && label != m_mantissaLabel &&
            label != m_binaryLabel && label != m_hexLabel &&
            label != m_binSignLabel && label != m_binExponentLabel && label != m_binMantissaLabel &&
            label != m_floatResultLabel && label != m_hexResultLabel)
            label->setStyleSheet(frmS);

    m_titleLabel->setStyleSheet(ttlS);
}

