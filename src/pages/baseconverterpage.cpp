#include "baseconverterpage.h"
#include "core/baseconvertercore.h"
#include "ui/themecolors.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QScrollArea>
#include <QSizePolicy>
#include <QFont>

// ─── BitButton ───────────────────────────────────────────────────────────────

BitButton::BitButton(int bitIndex, QWidget* parent)
    : QPushButton(parent), m_bitIndex(bitIndex) {
    setFixedSize(14, 14);
    setCheckable(false);
    refresh();
    connect(this, &QPushButton::clicked, this, [this]{
        m_state = !m_state;
        refresh();
        emit toggled2(m_bitIndex, m_state);
    });
}

void BitButton::setState(bool on) {
    m_state = on;
    refresh();
}

void BitButton::setDark(bool dark) {
    m_dark = dark;
    refresh();
}

void BitButton::refresh() {
    setText(m_state ? "1" : "0");
    setStyleSheet(ThemeColors::baseBitButtonStyle(m_dark, m_state));
}

// ─── BaseConverterPage ────────────────────────────────────────────────────────

BaseConverterPage::BaseConverterPage(QWidget* parent) : QWidget(parent) {
    setupUI();
}

void BaseConverterPage::setupUI() {
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
    root->setContentsMargins(10, 10, 10, 10);

    // ── Controls bar ─────────────────────────────────────────────────────────
    auto* ctrlRow = new QHBoxLayout();
    m_wLabel = new QLabel("Word width:", this);
    m_wLabel->setStyleSheet("font-size:13px;");
    m_widthCombo = new QComboBox(this);
    m_widthCombo->addItems({"8-bit", "16-bit", "32-bit", "64-bit"});
    m_widthCombo->setCurrentIndex(2);
    m_signedCheck = new QCheckBox("Signed", this);
    m_signedCheck->setStyleSheet("font-size:13px;");
    ctrlRow->addWidget(m_wLabel);
    ctrlRow->addWidget(m_widthCombo);
    ctrlRow->addSpacing(20);
    ctrlRow->addWidget(m_signedCheck);
    ctrlRow->addStretch();
    root->addLayout(ctrlRow);

    // ── Input fields ─────────────────────────────────────────────────────────
    auto* inputGrid = new QGridLayout();
    inputGrid->setSpacing(6);

    struct { const char* label; QLineEdit** edit; const char* placeholder; } fields[] = {
        {"HEX", &m_hexEdit, "e.g. DEADBEEF"},
        {"DEC", &m_decEdit, "e.g. 3735928559"},
        {"BIN", &m_binEdit, "e.g. 1101..."},
        {"OCT", &m_octEdit, "e.g. 33653337357"},
    };
    for (int i = 0; i < 4; ++i) {
        m_fieldLabels[i] = new QLabel(fields[i].label, this);
        m_fieldLabels[i]->setStyleSheet("font-size:13px;font-weight:bold;");
        m_fieldLabels[i]->setFixedWidth(60);
        *fields[i].edit = new QLineEdit(this);
        (*fields[i].edit)->setPlaceholderText(fields[i].placeholder);
        (*fields[i].edit)->setFont(QFont("Consolas", 14));
        inputGrid->addWidget(m_fieldLabels[i], i, 0);
        inputGrid->addWidget(*fields[i].edit, i, 1);
    }
    root->addLayout(inputGrid);

    // ── Register / Bit Viewer ─────────────────────────────────────────────────
    m_regGroup = new QGroupBox("Register View", this);
    m_regGroup->setStyleSheet(ThemeColors::baseGroupStyle(true));
    auto* regLayout = new QVBoxLayout(m_regGroup);
    regLayout->setSpacing(4);

    // Bit buttons will be created on first updateAll / width change
    // We use a grid inside a scroll area concept - just a flow widget here:
    auto* bitArea = new QWidget(this);
    auto* bitGrid = new QGridLayout(bitArea);
    bitGrid->setSpacing(2);
    bitGrid->setContentsMargins(4, 4, 4, 4);

    // Create 64 bit buttons (show only m_wordBits)
    for (int i = 63; i >= 0; --i) {
        auto* bb = new BitButton(i, bitArea);
        m_bitBtns.push_back(bb);
        connect(bb, &BitButton::toggled2, this, &BaseConverterPage::onBitToggled);
        // Position: bit 63 at left, bit 0 at right
        // Row 0 = bits 63..32 (if 64-bit), Row 1 = bits 31..0
        // For 32-bit: row 0 = bits 31..16, row 1 = bits 15..0
        // We'll layout dynamically - use a flat approach: two rows of 16
        int flatPos = 63 - i; // 0=bit63, 63=bit0
        int row = flatPos / 16;
        int col = flatPos % 16;
        bitGrid->addWidget(bb, row * 2, col); // *2 for index labels

        // Bit index label below button
        auto* idxLbl = new QLabel(QString::number(i), bitArea);
        idxLbl->setAlignment(Qt::AlignCenter);
        idxLbl->setStyleSheet("font-size:9px;");
        idxLbl->setFixedWidth(14);
        bitGrid->addWidget(idxLbl, row * 2 + 1, col);
    }

    regLayout->addWidget(bitArea);

    // Byte values row
    auto* byteRow = new QHBoxLayout();
    byteRow->addWidget(new QLabel("Bytes:", this));
    for (int i = 7; i >= 0; --i) {
        auto* bl = new QLabel("00", this);
        bl->setAlignment(Qt::AlignCenter);
        bl->setFixedWidth(32);
        bl->setToolTip(QString("Byte %1").arg(i));
        m_byteLabels[i] = bl;
        byteRow->addWidget(new QLabel(QString("B%1:").arg(i), this));
        byteRow->addWidget(bl);
    }
    byteRow->addStretch();
    regLayout->addLayout(byteRow);
    root->addWidget(m_regGroup);

    // ── Info section ─────────────────────────────────────────────────────────
    m_infoGroup = new QGroupBox("Interpretation", this);
    m_infoGroup->setStyleSheet(m_regGroup->styleSheet());
    auto* infoGrid = new QGridLayout(m_infoGroup);
    infoGrid->setSpacing(4);

    auto makeLbl = [&](const QString& t) {
        auto* l = new QLabel(t, this);
        l->setStyleSheet("font-size:12px;");
        return l;
    };
    auto makeVal = [&]() {
        auto* l = new QLabel("—", this);
        l->setFont(QFont("Consolas", 11));
        return l;
    };

    m_signedLabel   = makeVal();
    m_unsignedLabel = makeVal();
    m_hexInfoLabel  = makeVal();
    m_floatLabel    = makeVal();

    infoGrid->addWidget(makeLbl("Signed:"),   0, 0); infoGrid->addWidget(m_signedLabel,   0, 1);
    infoGrid->addWidget(makeLbl("Unsigned:"), 0, 2); infoGrid->addWidget(m_unsignedLabel, 0, 3);
    infoGrid->addWidget(makeLbl("Hex:"),      1, 0); infoGrid->addWidget(m_hexInfoLabel,  1, 1);
    infoGrid->addWidget(makeLbl("Float32:"),  1, 2); infoGrid->addWidget(m_floatLabel,    1, 3);
    root->addWidget(m_infoGroup);
    root->addStretch();

    // Connect signals
    connect(m_hexEdit, &QLineEdit::textEdited, this, &BaseConverterPage::onHexChanged);
    connect(m_decEdit, &QLineEdit::textEdited, this, &BaseConverterPage::onDecChanged);
    connect(m_binEdit, &QLineEdit::textEdited, this, &BaseConverterPage::onBinChanged);
    connect(m_octEdit, &QLineEdit::textEdited, this, &BaseConverterPage::onOctChanged);
    connect(m_widthCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &BaseConverterPage::onWordWidthChanged);
    connect(m_signedCheck, &QCheckBox::toggled, this, &BaseConverterPage::onSignedToggled);

    // Initial state
    onWordWidthChanged(2); // 32-bit
    updateAll(0);

    scroll->setWidget(content);
    outer->addWidget(scroll);
}

void BaseConverterPage::onWordWidthChanged(int index) {
    const int widths[] = {8, 16, 32, 64};
    m_wordBits = widths[index];

    // Show/hide bit buttons
    for (int i = 0; i < 64; ++i) {
        int bitIdx = m_bitBtns[63 - i]->bitIndex(); // m_bitBtns[0] = bit63
        bool visible = (bitIdx < m_wordBits);
        m_bitBtns[63 - i]->setVisible(visible);
        // Also hide its index label (it's at row*2+1 in grid)
    }
    // Show/hide byte labels
    int numBytes = m_wordBits / 8;
    for (int i = 0; i < 8; ++i) {
        m_byteLabels[i]->setVisible(i < numBytes);
    }

    m_value = m_value & ((m_wordBits == 64) ? ~0ULL : ((1ULL << m_wordBits) - 1));
    updateAll(m_value);
}

void BaseConverterPage::onSignedToggled(bool checked) {
    m_signed = checked;
    updateInfoLabels(m_value);
}

void BaseConverterPage::updateAll(unsigned long long value, QLineEdit* skip) {
    if (m_updating) return;
    m_updating = true;
    m_value = BaseConverterCore::applyMask(value, m_wordBits);

    if (m_hexEdit != skip) m_hexEdit->setText(QString::number(m_value, 16).toUpper());
    if (m_decEdit != skip) m_decEdit->setText(QString::number(m_value));
    if (m_binEdit != skip) m_binEdit->setText(BaseConverterCore::formatBinarySpaced(m_value, m_wordBits));
    if (m_octEdit != skip) m_octEdit->setText(QString::number(m_value, 8));

    updateBitButtons(m_value);
    updateInfoLabels(m_value);
    m_updating = false;
}

void BaseConverterPage::updateBitButtons(unsigned long long value) {
    for (auto* bb : m_bitBtns) {
        if (bb->bitIndex() < m_wordBits) {
            bb->setState((value >> bb->bitIndex()) & 1);
        }
    }
}

void BaseConverterPage::updateInfoLabels(unsigned long long value) {
    // Unsigned
    m_unsignedLabel->setText(QString::number(value));
    // Hex
    m_hexInfoLabel->setText(BaseConverterCore::hexWithPadding(value, m_wordBits));

    // Signed interpretation
    long long sval = BaseConverterCore::signedValue(value, m_wordBits);
    m_signedLabel->setText(QString::number(sval));

    // Float (only meaningful for 32-bit)
    if (m_wordBits == 32) {
        m_floatLabel->setText(BaseConverterCore::float32String(value));
    } else {
        m_floatLabel->setText(m_wordBits == 64 ? "(use 32-bit)" : "—");
    }

    // Byte labels
    for (int i = 0; i < m_wordBits / 8; ++i) {
        m_byteLabels[i]->setText(BaseConverterCore::byteHex(value, i));
        m_byteLabels[i]->setToolTip(BaseConverterCore::byteTooltip(value, i));
    }
}

void BaseConverterPage::onBitToggled(int bit, bool state) {
    if (state) m_value |=  (1ULL << bit);
    else        m_value &= ~(1ULL << bit);
    updateAll(m_value);
}

void BaseConverterPage::onHexChanged() {
    unsigned long long v = 0;
    if (BaseConverterCore::tryParse(m_hexEdit->text(), 16, v)) updateAll(v, m_hexEdit);
}

void BaseConverterPage::onDecChanged() {
    unsigned long long v = 0;
    if (BaseConverterCore::tryParse(m_decEdit->text(), 10, v)) updateAll(v, m_decEdit);
}

void BaseConverterPage::onBinChanged() {
    unsigned long long v = 0;
    if (BaseConverterCore::tryParse(m_binEdit->text(), 2, v)) updateAll(v, m_binEdit);
}

void BaseConverterPage::onOctChanged() {
    unsigned long long v = 0;
    if (BaseConverterCore::tryParse(m_octEdit->text(), 8, v)) updateAll(v, m_octEdit);
}

void BaseConverterPage::applyTheme(bool dark) {
    const QString editS = ThemeColors::baseEditStyle(dark);
    const QString valS  = ThemeColors::baseValueStyle(dark);
    const QString grpS  = ThemeColors::baseGroupStyle(dark);
    const QString fldS  = ThemeColors::baseFieldLabelStyle(dark);

    m_hexEdit->setStyleSheet(editS);
    m_decEdit->setStyleSheet(editS);
    m_binEdit->setStyleSheet(editS);
    m_octEdit->setStyleSheet(editS);

    m_signedLabel->setStyleSheet(valS);
    m_unsignedLabel->setStyleSheet(valS);
    m_hexInfoLabel->setStyleSheet(valS);
    m_floatLabel->setStyleSheet(valS);

    for (int i = 0; i < 8; ++i)
        m_byteLabels[i]->setStyleSheet(valS);

    for (int i = 0; i < 4; ++i)
        m_fieldLabels[i]->setStyleSheet(fldS);

    m_regGroup->setStyleSheet(grpS);
    m_infoGroup->setStyleSheet(grpS);

    for (auto* bb : m_bitBtns)
        bb->setDark(dark);
}
