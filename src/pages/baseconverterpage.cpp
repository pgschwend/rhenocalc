#include "baseconverterpage.h"
#include "core/baseconverter.h"
#include "ui/themecolors.h"
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QScrollArea>
#include <QSizePolicy>
#include <QFont>


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
        {"DEC", &m_decEdit, "e.g. 3735928559"},
        {"HEX", &m_hexEdit, "e.g. ABCDEF"},
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
    m_regGroup->setStyleSheet(Rheno::UI::baseGroupStyle(true));
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
        bitGrid->addWidget(bb, row * 2 + 1, col); // *2 for index labels

        // Bit index label above button
        auto* idxLbl = new QLabel(QString::number(i), bitArea);
        m_bitLabels.push_back(idxLbl);
        idxLbl->setAlignment(Qt::AlignCenter);
        idxLbl->setStyleSheet("font-size:11px; margin-top:-2px;");
        idxLbl->setFixedWidth(14);
        bitGrid->addWidget(idxLbl, row * 2, col);
    }

    regLayout->addWidget(bitArea);

    // Byte values - labels above, values below
    auto* byteGrid = new QGridLayout();
    byteGrid->setSpacing(4);
    for (int i = 7; i >= 0; --i) {
        int col = 7 - i;  // B7 at left (col 0), B0 at right (col 7)

        auto* label = new QLabel(QString("B%1").arg(i), this);
        label->setAlignment(Qt::AlignCenter);
        label->setStyleSheet("font-size:11px;");
        m_byteTitles[i] = label;
        byteGrid->addWidget(label, 0, col);

        auto* bl = new QLabel("00", this);
        bl->setAlignment(Qt::AlignCenter);
        bl->setFixedWidth(32);
        bl->setToolTip(QString("Byte %1").arg(i));
        m_byteLabels[i] = bl;
        byteGrid->addWidget(bl, 1, col);
    }
    regLayout->addLayout(byteGrid);
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

    // Install event filters to redirect keyboard events to page
    m_hexEdit->installEventFilter(this);
    m_decEdit->installEventFilter(this);
    m_binEdit->installEventFilter(this);
    m_octEdit->installEventFilter(this);
    m_widthCombo->installEventFilter(this);
    m_signedCheck->installEventFilter(this);

    // Initial state
    onWordWidthChanged(2); // 32-bit
    updateAll(0);

    scroll->setWidget(content);
    outer->addWidget(scroll);

    // Enable keyboard input
    setFocusPolicy(Qt::StrongFocus);
}

void BaseConverterPage::onWordWidthChanged(int index) {
    const int widths[] = {8, 16, 32, 64};
    m_wordBits = widths[index];

    // Show/hide bit buttons
    for (int i = 0; i < 64; ++i) {
        int bitIdx = m_bitBtns[63 - i]->bitIndex(); // m_bitBtns[0] = bit63
        bool visible = (bitIdx < m_wordBits);
        m_bitLabels[63 - i]->setVisible(visible || (i<16)); // Show always at least 16 bitLabels
        m_bitBtns[63 - i]->setVisible(visible);
    }
    // Show/hide byte labels
    int numBytes = m_wordBits / 8;
    for (int i = 0; i < 8; ++i) {
        if (i < numBytes) {
            m_byteTitles[i]->setText(QString("B%1").arg(i));
        } else {
            m_byteTitles[i]->setText("");
        }

        m_byteLabels[i]->setVisible(i < numBytes);
    }

    m_value = m_value & ((m_wordBits == 64) ? ~0ULL : ((1ULL << m_wordBits) - 1));
    updateAll(m_value);
}

void BaseConverterPage::onSignedToggled(bool checked) {
    m_signed = checked;
    updateAll(m_value);
}

void BaseConverterPage::updateAll(unsigned long long value, QLineEdit* skip) {
    if (m_updating) return;
    m_updating = true;
    m_value = Rheno::Core::applyMask(value, m_wordBits);

    // Track if values are at 0 for ESC handling
    m_isCleared = (m_value == 0);

    if (m_hexEdit != skip) m_hexEdit->setText(QString::number(m_value, 16).toUpper());
    if (m_decEdit != skip) {
        if (m_signed) {
            m_decEdit->setText(QString::number(Rheno::Core::signedValue(m_value, m_wordBits)));
        } else {
            m_decEdit->setText(QString::number(static_cast<qulonglong>(m_value)));
        }
    }
    if (m_binEdit != skip) m_binEdit->setText(Rheno::Core::formatBinarySpaced(m_value, m_wordBits));
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
    m_hexInfoLabel->setText(Rheno::Core::hexWithPadding(value, m_wordBits));

    // Signed interpretation
    long long sval = Rheno::Core::signedValue(value, m_wordBits);
    m_signedLabel->setText(QString::number(sval));

    // Float (only meaningful for 32-bit)
    if (m_wordBits == 32) {
        m_floatLabel->setText(Rheno::Core::float32String(value));
    } else {
        m_floatLabel->setText(m_wordBits == 64 ? "(use 32-bit)" : "—");
    }

    // Byte labels
    for (int i = 0; i < m_wordBits / 8; ++i) {
        m_byteLabels[i]->setText(Rheno::Core::byteHex(value, i));
        m_byteLabels[i]->setToolTip(Rheno::Core::byteTooltip(value, i));
    }
}


void BaseConverterPage::onBitToggled(int bit, bool state) {
    if (state) m_value |=  (1ULL << bit);
    else        m_value &= ~(1ULL << bit);
    updateAll(m_value);
}

void BaseConverterPage::onHexChanged() {
    if (m_updating) return;
    unsigned long long v = 0;
    if (Rheno::Core::tryParse(m_hexEdit->text(), 16, v)) updateAll(v, m_hexEdit);
}

void BaseConverterPage::onDecChanged() {
    if (m_updating) return;
    unsigned long long v = 0;
    if (Rheno::Core::tryParse(m_decEdit->text(), 10, v)) updateAll(v, m_decEdit);
}

void BaseConverterPage::onBinChanged() {
    if (m_updating) return;
    unsigned long long v = 0;
    if (Rheno::Core::tryParse(m_binEdit->text(), 2, v)) updateAll(v, m_binEdit);
}

void BaseConverterPage::onOctChanged() {
    if (m_updating) return;
    unsigned long long v = 0;
    if (Rheno::Core::tryParse(m_octEdit->text(), 8, v)) updateAll(v, m_octEdit);
}

void BaseConverterPage::applyTheme(bool dark) {
    const QString editS = Rheno::UI::baseEditStyle(dark);
    const QString valS  = Rheno::UI::baseValueStyle(dark);
    const QString grpS  = Rheno::UI::baseGroupStyle(dark);
    const QString fldS  = Rheno::UI::baseFieldLabelStyle(dark);

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

// ─── Keyboard support ────────────────────────────────────────────────────────
void BaseConverterPage::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    // Set focus to DEC field when shown so user can immediately type numbers
    m_decEdit->setFocus();
    m_decEdit->selectAll();
}

bool BaseConverterPage::eventFilter(QObject* watched, QEvent* event) {
    // Redirect key events from input widgets to page for global shortcuts
    if (event->type() == QEvent::KeyPress) {
        auto* keyEvent = static_cast<QKeyEvent*>(event);
        const int key = keyEvent->key();
        const Qt::KeyboardModifiers mod = keyEvent->modifiers();

        // For line edits: allow normal text editing, but intercept global shortcuts
        if (qobject_cast<QLineEdit*>(watched)) {
            // Allow normal navigation in text fields
            if (key == Qt::Key_Left || key == Qt::Key_Right ||
                key == Qt::Key_Home || key == Qt::Key_End ||
                key == Qt::Key_Backspace || key == Qt::Key_Delete) {
                return QWidget::eventFilter(watched, event);
            }

            // Intercept Ctrl+X/D/B/O for focus switching
            if (mod == Qt::ControlModifier) {
                if (key == Qt::Key_X || key == Qt::Key_D ||
                    key == Qt::Key_B || key == Qt::Key_O ||
                    key == Qt::Key_1 || key == Qt::Key_2 ||
                    key == Qt::Key_3 || key == Qt::Key_4) {
                    keyPressEvent(keyEvent);
                    return true;
                }
            }

            // Intercept +/- for signed checkbox
            if (mod == Qt::NoModifier || mod == Qt::ShiftModifier) {
                if (key == Qt::Key_Plus || key == Qt::Key_Minus) {
                    keyPressEvent(keyEvent);
                    return true;
                }
            }

            // Intercept ESC
            if (key == Qt::Key_Escape) {
                keyPressEvent(keyEvent);
                return true;
            }

            return QWidget::eventFilter(watched, event);
        }

        // For combobox and checkbox: allow normal interaction but intercept shortcuts
        if (qobject_cast<QComboBox*>(watched) || qobject_cast<QCheckBox*>(watched)) {
            // Allow normal navigation
            if (key == Qt::Key_Up || key == Qt::Key_Down ||
                key == Qt::Key_Return || key == Qt::Key_Enter ||
                key == Qt::Key_Space) {
                return QWidget::eventFilter(watched, event);
            }

            // Intercept global shortcuts
            if (mod == Qt::ControlModifier) {
                if (key == Qt::Key_X || key == Qt::Key_D ||
                    key == Qt::Key_B || key == Qt::Key_O ||
                    key == Qt::Key_1 || key == Qt::Key_2 ||
                    key == Qt::Key_3 || key == Qt::Key_4) {
                    keyPressEvent(keyEvent);
                    return true;
                }
            }

            // Intercept +/-
            if (mod == Qt::NoModifier || mod == Qt::ShiftModifier) {
                if (key == Qt::Key_Plus || key == Qt::Key_Minus) {
                    keyPressEvent(keyEvent);
                    return true;
                }
            }

            // Intercept ESC
            if (key == Qt::Key_Escape) {
                keyPressEvent(keyEvent);
                return true;
            }

            return QWidget::eventFilter(watched, event);
        }
    }

    return QWidget::eventFilter(watched, event);
}

void BaseConverterPage::keyPressEvent(QKeyEvent* event) {
    const int key = event->key();
    const Qt::KeyboardModifiers mod = event->modifiers();

    // ── Ctrl+X/D/B/O: Focus switching between input fields ───────────────────
    if (mod == Qt::ControlModifier) {
        switch (key) {
        case Qt::Key_X:  // Ctrl+X → Focus HEX field
            m_hexEdit->setFocus();
            m_hexEdit->selectAll();
            event->accept();
            return;
        case Qt::Key_D:  // Ctrl+D → Focus DEC field
            m_decEdit->setFocus();
            m_decEdit->selectAll();
            event->accept();
            return;
        case Qt::Key_B:  // Ctrl+B → Focus BIN field
            m_binEdit->setFocus();
            m_binEdit->selectAll();
            event->accept();
            return;
        case Qt::Key_O:  // Ctrl+O → Focus OCT field
            m_octEdit->setFocus();
            m_octEdit->selectAll();
            event->accept();
            return;

        // ── Ctrl+1-4: Word width switching ───────────────────────────────────
        case Qt::Key_1:
            m_widthCombo->setCurrentIndex(0);  // 8-bit
            event->accept();
            return;
        case Qt::Key_2:
            m_widthCombo->setCurrentIndex(1);  // 16-bit
            event->accept();
            return;
        case Qt::Key_3:
            m_widthCombo->setCurrentIndex(2);  // 32-bit
            event->accept();
            return;
        case Qt::Key_4:
            m_widthCombo->setCurrentIndex(3);  // 64-bit
            event->accept();
            return;

        default:
            break;
        }
    }

    // ── + / - : Toggle signed checkbox ───────────────────────────────────────
    if (mod == Qt::NoModifier || mod == Qt::ShiftModifier) {
        if (key == Qt::Key_Minus) {
            m_signedCheck->setChecked(true);
            event->accept();
            return;
        }
        if (key == Qt::Key_Plus) {
            m_signedCheck->setChecked(false);
            event->accept();
            return;
        }
    }

    // ── ESC: Clear values or close app ───────────────────────────────────────
    if (key == Qt::Key_Escape) {
        // First ESC: Clear values to 0 (if not already 0)
        if (!m_isCleared || m_value != 0) {
            updateAll(0);
            // m_isCleared is set by updateAll(0)
            event->accept();  // Important: accept to prevent propagation
            return;
        }

        // Second ESC (when already cleared): Close app if setting enabled
        if (QWidget* mainWin = window()) {
            QSettings settings("RhenoCalc", "RhenoCalc");
            if (settings.value("closeWithEscCheck", false).toBool()) {
                event->accept();
                mainWin->close();
                return;
            }
        }
        // If setting is disabled, don't close but accept the event
        event->accept();
        return;
    }

    QWidget::keyPressEvent(event);
}

