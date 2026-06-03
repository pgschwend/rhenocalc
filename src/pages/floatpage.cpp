#include "floatpage.h"
#include "ui/themecolors.h"

#include <QComboBox>
#include <QDoubleValidator>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QtMath>
#include <cmath>
#include <cstdint>
#include <limits>

namespace {

// Float type parameters: {totalBits, exponentBits, mantissaBits, bias}
struct FloatFormat {
    int totalBits;
    int expBits;
    int mantBits;
    int bias;
};

FloatFormat getFormat(const QString& typeName) {
    if (typeName == "float16 (Half)")
        return {16, 5, 10, 15};
    if (typeName == "float32 (Single)")
        return {32, 8, 23, 127};
    if (typeName == "float64 (Double)")
        return {64, 11, 52, 1023};
    return {32, 8, 23, 127}; // Default to float32
}

} // namespace

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

QString FloatPage::doubleToIEEE(double value, int totalBits, int expBits, int mantBits) {
    const int bias = (1 << (expBits - 1)) - 1;

    // Handle special cases
    if (std::isnan(value)) {
        // NaN: all exponent bits 1, non-zero mantissa
        QString result(totalBits, '0');
        for (int i = 1; i <= expBits; ++i)
            result[i] = '1';
        result[totalBits - 1] = '1'; // non-zero mantissa
        return result;
    }

    if (std::isinf(value)) {
        // Infinity: all exponent bits 1, zero mantissa
        QString result(totalBits, '0');
        if (value < 0) result[0] = '1';
        for (int i = 1; i <= expBits; ++i)
            result[i] = '1';
        return result;
    }

    if (value == 0.0) {
        QString result(totalBits, '0');
        // Check for -0.0
        if (std::signbit(value))
            result[0] = '1';
        return result;
    }

    QString result(totalBits, '0');

    // Sign bit
    if (value < 0) {
        result[0] = '1';
        value = -value;
    }

    // Calculate exponent and mantissa
    int exponent = 0;
    double mantissa = std::frexp(value, &exponent);
    // frexp returns mantissa in [0.5, 1.0), we need [1.0, 2.0)
    mantissa *= 2.0;
    exponent -= 1;

    // Bias the exponent
    int biasedExp = exponent + bias;

    // Check for overflow (infinity)
    if (biasedExp >= (1 << expBits) - 1) {
        // Return infinity
        for (int i = 1; i <= expBits; ++i)
            result[i] = '1';
        return result;
    }

    // Check for underflow (denormalized or zero)
    if (biasedExp <= 0) {
        // Denormalized number
        biasedExp = 0;
        mantissa = value / std::pow(2.0, 1 - bias);
        // For denormals, there's no implicit leading 1
    } else {
        // Normalized: remove implicit leading 1
        mantissa -= 1.0;
    }

    // Write exponent bits
    for (int i = expBits - 1; i >= 0; --i) {
        result[1 + (expBits - 1 - i)] = (biasedExp & (1 << i)) ? '1' : '0';
    }

    // Write mantissa bits
    for (int i = 0; i < mantBits; ++i) {
        mantissa *= 2.0;
        if (mantissa >= 1.0) {
            result[1 + expBits + i] = '1';
            mantissa -= 1.0;
        }
    }

    return result;
}

double FloatPage::ieeeToDouble(const QString& bits, int totalBits, int expBits, int mantBits) {
    if (bits.length() != totalBits)
        return std::numeric_limits<double>::quiet_NaN();

    const int bias = (1 << (expBits - 1)) - 1;

    // Extract sign
    int sign = (bits[0] == '1') ? -1 : 1;

    // Extract exponent
    int exponent = 0;
    for (int i = 1; i <= expBits; ++i) {
        exponent = (exponent << 1) | (bits[i] == '1' ? 1 : 0);
    }

    // Extract mantissa
    double mantissa = 0.0;
    double fraction = 0.5;
    for (int i = 1 + expBits; i < totalBits; ++i) {
        if (bits[i] == '1')
            mantissa += fraction;
        fraction /= 2.0;
    }

    // Special cases
    const int maxExp = (1 << expBits) - 1;

    if (exponent == 0) {
        // Denormalized or zero
        if (mantissa == 0.0)
            return sign > 0 ? 0.0 : -0.0;
        // Denormalized: no implicit 1
        return sign * mantissa * std::pow(2.0, 1 - bias);
    }

    if (exponent == maxExp) {
        // Infinity or NaN
        if (mantissa == 0.0)
            return sign > 0 ? std::numeric_limits<double>::infinity()
                           : -std::numeric_limits<double>::infinity();
        return std::numeric_limits<double>::quiet_NaN();
    }

    // Normalized number: add implicit 1
    mantissa += 1.0;
    return sign * mantissa * std::pow(2.0, exponent - bias);
}

QString FloatPage::formatBinaryString(const QString& bits, int expBits, int mantBits) {
    if (bits.length() != 1 + expBits + mantBits)
        return bits;

    // Format: S EEEE...E MMM...M
    return bits.left(1) + " " + bits.mid(1, expBits) + " " + bits.mid(1 + expBits);
}

void FloatPage::floatToBinary() {
    QString text = m_floatInput->text().trimmed();
    if (text.isEmpty()) {
        m_signLabel->setText("—");
        m_exponentLabel->setText("—");
        m_mantissaLabel->setText("—");
        m_binaryLabel->setText("—");
        m_hexLabel->setText("—");
        return;
    }

    bool ok = false;
    double value = text.toDouble(&ok);
    if (!ok) {
        // Try with comma as decimal separator
        QString normalized = text;
        normalized.replace(',', '.');
        value = normalized.toDouble(&ok);
    }

    if (!ok) {
        m_signLabel->setText("Invalid");
        m_exponentLabel->setText("—");
        m_mantissaLabel->setText("—");
        m_binaryLabel->setText("—");
        m_hexLabel->setText("—");
        return;
    }

    FloatFormat fmt = getFormat(m_floatTypeCombo->currentText());
    QString binary = doubleToIEEE(value, fmt.totalBits, fmt.expBits, fmt.mantBits);

    // Extract components
    QString sign = binary.left(1);
    QString exponent = binary.mid(1, fmt.expBits);
    QString mantissa = binary.mid(1 + fmt.expBits);

    m_signLabel->setText(sign + (sign == "0" ? " (+)" : " (−)"));
    m_exponentLabel->setText(exponent + " (" + QString::number(exponent.toInt(nullptr, 2)) + ")");
    m_mantissaLabel->setText(mantissa);
    m_binaryLabel->setText(formatBinaryString(binary, fmt.expBits, fmt.mantBits));

    // Convert to hex
    QString hex;
    for (int i = 0; i < binary.length(); i += 4) {
        QString nibble = binary.mid(i, 4);
        hex += QString::number(nibble.toInt(nullptr, 2), 16).toUpper();
    }
    m_hexLabel->setText("0x" + hex);
}

void FloatPage::binaryToFloat() {
    QString input = m_binaryInput->text().trimmed();
    // Remove spaces and other separators
    input.remove(' ').remove('-').remove('_');

    FloatFormat fmt = getFormat(m_binTypeCombo->currentText());

    if (input.isEmpty() || input.length() != fmt.totalBits) {
        QString expected = QString("Need %1 bits").arg(fmt.totalBits);
        if (input.isEmpty()) expected = "—";
        m_binSignLabel->setText(expected);
        m_binExponentLabel->setText("—");
        m_binMantissaLabel->setText("—");
        m_floatResultLabel->setText("—");
        m_hexResultLabel->setText("—");
        return;
    }

    // Validate binary string
    for (QChar c : input) {
        if (c != '0' && c != '1') {
            m_binSignLabel->setText("Invalid binary");
            m_binExponentLabel->setText("—");
            m_binMantissaLabel->setText("—");
            m_floatResultLabel->setText("—");
            m_hexResultLabel->setText("—");
            return;
        }
    }

    // Extract components
    QString sign = input.left(1);
    QString exponent = input.mid(1, fmt.expBits);
    QString mantissa = input.mid(1 + fmt.expBits);

    m_binSignLabel->setText(sign + (sign == "0" ? " (+)" : " (−)"));
    m_binExponentLabel->setText(exponent + " (" + QString::number(exponent.toInt(nullptr, 2)) + ")");
    m_binMantissaLabel->setText(mantissa);

    // Convert to double
    double result = ieeeToDouble(input, fmt.totalBits, fmt.expBits, fmt.mantBits);

    if (std::isnan(result)) {
        m_floatResultLabel->setText("NaN");
    } else if (std::isinf(result)) {
        m_floatResultLabel->setText(result > 0 ? "+∞" : "−∞");
    } else {
        // Use enough precision based on format
        int precision = (fmt.totalBits == 16) ? 4 : (fmt.totalBits == 32) ? 9 : 17;
        m_floatResultLabel->setText(QString::number(result, 'g', precision));
    }

    // Convert to hex
    QString hex;
    for (int i = 0; i < input.length(); i += 4) {
        QString nibble = input.mid(i, 4);
        hex += QString::number(nibble.toInt(nullptr, 2), 16).toUpper();
    }
    m_hexResultLabel->setText("0x" + hex);
}

void FloatPage::applyTheme(bool dark) {
    m_isDark = dark;

    const QString grpS = ThemeColors::unitGroupStyle(dark);
    const QString fldS = ThemeColors::unitFieldStyle(dark);
    const QString resS = ThemeColors::unitResultStyle(dark);
    const QString ttlS = ThemeColors::unitTitleStyle(dark);
    const QString frmS = ThemeColors::unitFormulaStyle(dark);

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

