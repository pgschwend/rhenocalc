#include "baseconverterpage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QFont>
#include <cstring>
#include <cmath>

// ─── BitButton ───────────────────────────────────────────────────────────────

BitButton::BitButton(int bitIndex, QWidget* parent)
    : QPushButton(parent), m_bitIndex(bitIndex) {
    setFixedSize(22, 28);
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
    if (m_dark) {
        setStyleSheet(m_state
            ? "QPushButton{background:#707070;color:white;font-size:11px;font-weight:bold;border-radius:3px;border:1px solid #999;}QPushButton:hover{background:#888888;}"
            : "QPushButton{background:#484848;color:#cccccc;font-size:11px;border-radius:3px;border:1px solid #666;}QPushButton:hover{background:#606060;}");
    } else {
        setStyleSheet(m_state
            ? "QPushButton{background:#3d5aaa;color:white;font-size:11px;font-weight:bold;border-radius:3px;border:1px solid #8899cc;}QPushButton:hover{background:#4d6abf;}"
            : "QPushButton{background:#eaecf5;color:#4455aa;font-size:11px;border-radius:3px;border:1px solid #c5cbdd;}QPushButton:hover{background:#d8dcee;}");
    }
}

// ─── BaseConverterPage ────────────────────────────────────────────────────────

BaseConverterPage::BaseConverterPage(QWidget* parent) : QWidget(parent) {
    setupUI();
}

void BaseConverterPage::setupUI() {
    auto* root = new QVBoxLayout(this);
    root->setSpacing(10);
    root->setContentsMargins(14, 14, 14, 14);

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
        {"HEX (0x)", &m_hexEdit, "e.g. DEADBEEF"},
        {"DEC",      &m_decEdit, "e.g. 3735928559"},
        {"BIN",      &m_binEdit, "e.g. 1101..."},
        {"OCT",      &m_octEdit, "e.g. 33653337357"},
    };
    for (int i = 0; i < 4; ++i) {
        m_fieldLabels[i] = new QLabel(fields[i].label, this);
        m_fieldLabels[i]->setStyleSheet("font-size:13px;font-weight:bold;");
        m_fieldLabels[i]->setFixedWidth(90);
        *fields[i].edit = new QLineEdit(this);
        (*fields[i].edit)->setPlaceholderText(fields[i].placeholder);
        (*fields[i].edit)->setFont(QFont("Consolas", 14));
        inputGrid->addWidget(m_fieldLabels[i], i, 0);
        inputGrid->addWidget(*fields[i].edit, i, 1);
    }
    root->addLayout(inputGrid);

    // ── Register / Bit Viewer ─────────────────────────────────────────────────
    m_regGroup = new QGroupBox("Register View", this);
    m_regGroup->setStyleSheet("QGroupBox{color:#b5b5b5;font-size:13px;font-weight:bold;border:1px solid #444;border-radius:6px;margin-top:8px;padding-top:8px;}"
                             "QGroupBox::title{subcontrol-origin:margin;left:10px;}");
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
        idxLbl->setStyleSheet("color:#666;font-size:9px;");
        idxLbl->setFixedWidth(22);
        bitGrid->addWidget(idxLbl, row * 2 + 1, col);
    }

    regLayout->addWidget(bitArea);

    // Byte values row
    auto* byteRow = new QHBoxLayout();
    byteRow->addWidget(new QLabel("Bytes:", this));
    for (int i = 7; i >= 0; --i) {
        auto* bl = new QLabel("00", this);
        bl->setAlignment(Qt::AlignCenter);
        bl->setFixedWidth(44);
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

unsigned long long applyMask(unsigned long long val, int bits) {
    if (bits == 64) return val;
    return val & ((1ULL << bits) - 1);
}

void BaseConverterPage::updateAll(unsigned long long value, QLineEdit* skip) {
    if (m_updating) return;
    m_updating = true;
    m_value = applyMask(value, m_wordBits);

    if (m_hexEdit != skip) m_hexEdit->setText(QString::number(m_value, 16).toUpper());
    if (m_decEdit != skip) m_decEdit->setText(QString::number(m_value));
    if (m_binEdit != skip) {
        QString b = QString::number(m_value, 2);
        // Pad to word width
        while (b.length() < m_wordBits) b.prepend('0');
        // Insert spaces every 4 bits for readability
        QString spaced;
        for (int i = 0; i < b.length(); ++i) {
            if (i > 0 && (b.length() - i) % 4 == 0) spaced += ' ';
            spaced += b[i];
        }
        m_binEdit->setText(spaced);
    }
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
    m_hexInfoLabel->setText("0x" + QString::number(value, 16).toUpper().rightJustified(m_wordBits/4, '0'));

    // Signed interpretation
    long long sval = value;
    if (m_wordBits < 64) {
        long long signBit = 1LL << (m_wordBits - 1);
        if (value & (unsigned long long)signBit)
            sval = (long long)(value | ~((1ULL << m_wordBits) - 1));
    }
    m_signedLabel->setText(QString::number(sval));

    // Float (only meaningful for 32-bit)
    if (m_wordBits == 32) {
        quint32 v32 = (quint32)value;
        float f;
        std::memcpy(&f, &v32, 4);
        if (std::isnan(f)) m_floatLabel->setText("NaN");
        else if (std::isinf(f)) m_floatLabel->setText(f > 0 ? "+Inf" : "-Inf");
        else m_floatLabel->setText(QString::number((double)f, 'g', 8));
    } else {
        m_floatLabel->setText(m_wordBits == 64 ? "(use 32-bit)" : "—");
    }

    // Byte labels
    for (int i = 0; i < m_wordBits / 8; ++i) {
        quint8 byte = (quint8)((value >> (i * 8)) & 0xFF);
        m_byteLabels[i]->setText(QString("%1").arg(byte, 2, 16, QChar('0')).toUpper());
        m_byteLabels[i]->setToolTip(QString("Byte %1: 0x%2 = %3").arg(i).arg(byte, 2, 16, QChar('0')).toUpper().arg(byte));
    }
}

void BaseConverterPage::onBitToggled(int bit, bool state) {
    if (state) m_value |=  (1ULL << bit);
    else        m_value &= ~(1ULL << bit);
    updateAll(m_value);
}

void BaseConverterPage::onHexChanged() {
    QString s = m_hexEdit->text().trimmed().remove(' ');
    bool ok;
    unsigned long long v = s.toULongLong(&ok, 16);
    if (ok) updateAll(v, m_hexEdit);
}

void BaseConverterPage::onDecChanged() {
    QString s = m_decEdit->text().trimmed().remove(' ');
    bool ok;
    unsigned long long v = s.toULongLong(&ok, 10);
    if (!ok) { long long sv = s.toLongLong(&ok, 10); if (ok) v = (unsigned long long)sv; }
    if (ok) updateAll(v, m_decEdit);
}

void BaseConverterPage::onBinChanged() {
    QString s = m_binEdit->text().trimmed().remove(' ');
    bool ok;
    unsigned long long v = s.toULongLong(&ok, 2);
    if (ok) updateAll(v, m_binEdit);
}

void BaseConverterPage::onOctChanged() {
    QString s = m_octEdit->text().trimmed().remove(' ');
    bool ok;
    unsigned long long v = s.toULongLong(&ok, 8);
    if (ok) updateAll(v, m_octEdit);
}

void BaseConverterPage::applyTheme(bool dark) {
    const QString editS = dark
        ? "background:#444444;color:#f0f0f0;font-family:'Consolas','Courier New',monospace;font-size:16px;border:1px solid #666;border-radius:4px;padding:4px 8px;"
        : "background:#ffffff;color:#1a1a2e;font-family:'Consolas','Courier New',monospace;font-size:16px;border:1px solid #c5cbdd;border-radius:4px;padding:4px 8px;";
    const QString valS = dark
        ? "background:#444444;color:#f0f0f0;font-family:'Consolas';font-size:13px;border:1px solid #666;border-radius:3px;padding:2px 8px;"
        : "background:#f0f2fa;color:#1a1a2e;font-family:'Consolas';font-size:13px;border:1px solid #c5cbdd;border-radius:3px;padding:2px 8px;";
    const QString grpS = dark
        ? "QGroupBox{color:#b5b5b5;font-size:13px;font-weight:bold;border:1px solid #444;border-radius:6px;margin-top:8px;padding-top:8px;}QGroupBox::title{subcontrol-origin:margin;left:10px;}"
        : "QGroupBox{color:#3d5aaa;font-size:13px;font-weight:bold;border:1px solid #c5cbdd;border-radius:6px;margin-top:8px;padding-top:8px;}QGroupBox::title{subcontrol-origin:margin;left:10px;}";
    const QString fldS = dark ? "color:#aaa;font-size:13px;font-weight:bold;" : "color:#3d5aaa;font-size:13px;font-weight:bold;";

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
