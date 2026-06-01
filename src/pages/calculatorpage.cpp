#include "calculatorpage.h"
#include "ui/themecolors.h"
#include <QClipboard>
#include <QGuiApplication>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFont>

CalculatorPage::CalculatorPage(QWidget* parent) : QWidget(parent) {
    setupUI();
}

QPushButton* CalculatorPage::makeBtn(const QString& text, const QString& style) {
    auto* btn = new QPushButton(text, this);
    btn->setMinimumSize(34, 28);
    btn->setStyleSheet(style.isEmpty() ? ThemeColors::calcNumButton(true) : style);
    return btn;
}

void CalculatorPage::setupUI() {
    auto* root = new QVBoxLayout(this);
    root->setSizeConstraint(QLayout::SetNoConstraint);
    root->setSpacing(6);
    root->setContentsMargins(8, 8, 8, 8);

    // Top controls
    auto* topRow = new QHBoxLayout();
    m_baseLabel = new QLabel("Base:", this);
    m_baseLabel->setStyleSheet("font-size:13px;");
    m_baseCombo = new QComboBox(this);
    m_baseCombo->addItems({"Decimal (10)", "Hexadecimal (16)", "Binary (2)", "Octal (8)"});
    m_baseCombo->setMinimumWidth(90);

    m_wordLabel = new QLabel("Mode:", this);
    m_wordLabel->setStyleSheet("font-size:13px;");
    m_widthCombo = new QComboBox(this);
    m_widthCombo->addItems({"8-bit", "16-bit", "32-bit", "64-bit", "Scientific"});
    m_widthCombo->setCurrentIndex(4);

    topRow->addWidget(m_baseLabel);
    topRow->addWidget(m_baseCombo);
    topRow->addSpacing(16);
    topRow->addWidget(m_wordLabel);
    topRow->addWidget(m_widthCombo);
    topRow->addStretch();
    root->addLayout(topRow);

    // Expression label
    m_exprLabel = new QLabel("", this);
    m_exprLabel->setAlignment(Qt::AlignRight);
    root->addWidget(m_exprLabel);

    // Display
    m_display = new QLineEdit("0", this);
    m_display->setAlignment(Qt::AlignRight);
    m_display->setReadOnly(true);
    m_display->setFont(QFont("Consolas,Courier New,monospace", 20));
    m_display->setMinimumHeight(34);
    root->addWidget(m_display);

    // Keyboard shortcut hint bar
    m_hintLabel = new QLabel(
        "% MOD  |  & AND  |  | OR  |  ^ XOR |  ~ NOT  |\n< LSL  |  > LSR  |  Enter =  |  Esc AC/Close  |  ⌫ BS  |\nCtrl+D/H/B/O: Base  |  Ctrl+1–5: Mode  |  Ctrl+◀ ▶: Tab",
        this);
    m_hintLabel->setWordWrap(true);
    root->addWidget(m_hintLabel);

    // Button grid
    auto* grid = new QGridLayout();
    grid->setSpacing(4);

    // Row 0: Hex letters A-F + Backspace
    const char* hexLetters[] = {"A","B","C","D","E","F"};
    for (int i = 0; i < 6; ++i) {
        m_hexBtns[i] = makeBtn(hexLetters[i], ThemeColors::calcHexButton(true));
        connect(m_hexBtns[i], &QPushButton::clicked, this, &CalculatorPage::onDigitClicked);
        grid->addWidget(m_hexBtns[i], 0, i);
    }
    auto* bsBtn = makeBtn("⌫", ThemeColors::calcFuncButton(true));
    m_funcBtns << bsBtn;
    connect(bsBtn, &QPushButton::clicked, this, &CalculatorPage::onBackspaceClicked);
    grid->addWidget(bsBtn, 0, 6);

    // Row 1: Bitwise ops
    const char* bitOps[] = {"AND","OR","XOR","NOT","LSL","LSR","ROR","ROL"};
    // Split to 2 rows of 4
    for (int i = 0; i < 4; ++i) {
        auto* b = makeBtn(bitOps[i], ThemeColors::calcBitButton(true));
        b->setObjectName(bitOps[i]);
        m_bitOpBtns << b;
        connect(b, &QPushButton::clicked, this, &CalculatorPage::onBitwiseClicked);
        grid->addWidget(b, 1, i);
    }
    for (int i = 4; i < 8; ++i) {
        auto* b = makeBtn(bitOps[i], ThemeColors::calcBitButton(true));
        b->setObjectName(bitOps[i]);
        m_bitOpBtns << b;
        connect(b, &QPushButton::clicked, this, &CalculatorPage::onBitwiseClicked);
        grid->addWidget(b, 2, i-4);
    }
    // Row 2 right side: MOD, NEG, CLR, DEL
    auto* modBtn = makeBtn("MOD", ThemeColors::calcFuncButton(true));
    modBtn->setObjectName("MOD");
    m_funcBtns << modBtn;
    connect(modBtn, &QPushButton::clicked, this, &CalculatorPage::onOperatorClicked);
    grid->addWidget(modBtn, 1, 4);

    auto* invBtn = makeBtn("1/x", ThemeColors::calcFuncButton(true));
    invBtn->setObjectName("1/x");
    m_funcBtns << invBtn;
    connect(invBtn, &QPushButton::clicked, this, &CalculatorPage::onBitwiseClicked);
    grid->addWidget(invBtn, 1, 5);

    auto* clrBtn = makeBtn("CE", ThemeColors::calcClearButton(true));
    m_clearBtns << clrBtn;
    connect(clrBtn, &QPushButton::clicked, this, &CalculatorPage::onClearEntryOrAllClicked);
    grid->addWidget(clrBtn, 1, 6);

    auto* powBtn = makeBtn("x²", ThemeColors::calcFuncButton(true));
    powBtn->setObjectName("SQ");
    m_funcBtns << powBtn;
    connect(powBtn, &QPushButton::clicked, this, &CalculatorPage::onBitwiseClicked);
    grid->addWidget(powBtn, 3, 5);

    auto* sqrtBtn = makeBtn("√x", ThemeColors::calcFuncButton(true));
    sqrtBtn->setObjectName("SQRT");
    m_funcBtns << sqrtBtn;
    connect(sqrtBtn, &QPushButton::clicked, this, &CalculatorPage::onBitwiseClicked);
    grid->addWidget(sqrtBtn, 3, 6);

    auto* secondFuncBtn = makeBtn("2nd", ThemeColors::calcSecondFuncButton(true));
    m_clearBtns << secondFuncBtn;
    connect(secondFuncBtn, &QPushButton::clicked, this, []{});
    grid->addWidget(secondFuncBtn, 2, 6);

    // Digits + operators (rows 3-6)
    struct BtnDef { QString label; int row, col; QString style; };
    QList<BtnDef> defs = {
        {"7",3,0,ThemeColors::calcNumButton(true)},
        {"8",3,1,ThemeColors::calcNumButton(true)},
        {"9",3,2,ThemeColors::calcNumButton(true)},
        {"÷",3,3,ThemeColors::calcOpButton(true)},
        {"(",2,4,ThemeColors::calcFuncButton(true)},
        {")",2,5,ThemeColors::calcFuncButton(true)},
        {"π",3,4,ThemeColors::calcBitButton(true)},
        {"4",4,0,ThemeColors::calcNumButton(true)},
        {"5",4,1,ThemeColors::calcNumButton(true)},
        {"6",4,2,ThemeColors::calcNumButton(true)},
        {"×",4,3,ThemeColors::calcOpButton(true)},
        {"e",4,4,ThemeColors::calcFuncButton(true)},
        {"log",4,5,ThemeColors::calcFuncButton(true)},
        {"ln",4,6,ThemeColors::calcFuncButton(true)},
        {"1",5,0,ThemeColors::calcNumButton(true)},
        {"2",5,1,ThemeColors::calcNumButton(true)},
        {"3",5,2,ThemeColors::calcNumButton(true)},
        {"-",5,3,ThemeColors::calcOpButton(true)},
        {"MS",5,4,ThemeColors::calcFuncButton(true)},
        {"MR",5,5,ThemeColors::calcFuncButton(true)},
        {"MC",5,6,ThemeColors::calcFuncButton(true)},
        {"+/-",6,0,ThemeColors::calcFuncButton(true)},
        {"0",6,1,ThemeColors::calcNumButton(true)},
        {".",6,2,ThemeColors::calcNumButton(true)},
        {"+",6,3,ThemeColors::calcOpButton(true)},
        {"=",6,4,ThemeColors::calcEqButton(true)},
    };

    // Make = button span 3 cols
    for (auto& d : defs) {
        auto* b = makeBtn(d.label, d.style);
        if (d.label == "=") {
            b->setObjectName("=");
            m_eqBtn = b;
            grid->addWidget(b, d.row, d.col, 1, 3);
            connect(b, &QPushButton::clicked, this, &CalculatorPage::onEqualsClicked);
        } else if (d.label == "÷" || d.label == "×" || d.label == "+" || d.label == "-" || d.label == "MOD") {
            b->setObjectName(d.label);
            m_opBtns << b;
            connect(b, &QPushButton::clicked, this, &CalculatorPage::onOperatorClicked);
            grid->addWidget(b, d.row, d.col);
        } else if (d.label == "π" || d.label == "e") {
            b->setObjectName(d.label);
            m_funcBtns << b;
            connect(b, &QPushButton::clicked, this, &CalculatorPage::onPiClicked);
            grid->addWidget(b, d.row, d.col);
        } else if (d.label == "1/x" || d.label == "log" || d.label == "ln") {
            b->setObjectName(d.label);
            m_funcBtns << b;
            connect(b, &QPushButton::clicked, this, &CalculatorPage::onBitwiseClicked);
            grid->addWidget(b, d.row, d.col);
        } else if (d.label == "+/-") {
            m_funcBtns << b;
            connect(b, &QPushButton::clicked, this, &CalculatorPage::onNegateClicked);
            grid->addWidget(b, d.row, d.col);
        } else if (d.label == "MS" || d.label == "MR" || d.label == "MC") {
            b->setObjectName(d.label);
            m_funcBtns << b;
            connect(b, &QPushButton::clicked, this, &CalculatorPage::onBitwiseClicked);
            grid->addWidget(b, d.row, d.col);
        } else if (d.label == "(" || d.label == ")") {
            m_funcBtns << b;
            connect(b, &QPushButton::clicked, this, &CalculatorPage::onDigitClicked);
            grid->addWidget(b, d.row, d.col);
        } else {
            m_numBtns << b;
            connect(b, &QPushButton::clicked, this, &CalculatorPage::onDigitClicked);
            grid->addWidget(b, d.row, d.col);
        }
    }

    root->addLayout(grid);
    root->addStretch();

    connect(m_baseCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CalculatorPage::onBaseChanged);
    connect(m_widthCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CalculatorPage::onWordWidthChanged);

    // Apply initial word-width / big-mode setting (connect was made after setCurrentIndex)
    onWordWidthChanged(m_widthCombo->currentIndex());

    // Initially disable hex letters
    for (int i = 0; i < 6; ++i) m_hexBtns[i]->setEnabled(false);

    // Enable keyboard input
    setFocusPolicy(Qt::StrongFocus);
}

long long CalculatorPage::maskToWidth(long long val) {
    return CalculatorCore::maskToWidth(val, m_engine.wordBits());
}

QString CalculatorPage::toBaseString(long long val) {
    return CalculatorCore::toBaseString(val, m_engine.base(), m_engine.wordBits());
}

long long CalculatorPage::fromBaseString(const QString& s) {
    return CalculatorCore::fromBaseString(s, m_engine.base());
}

void CalculatorPage::updateDisplay() {
    m_display->setText(m_engine.displayText());
    m_exprLabel->setText(m_engine.expressionText());
    bool hexMode = (m_engine.base() == 16);
    for (int i = 0; i < 6; ++i) m_hexBtns[i]->setEnabled(hexMode);
}

void CalculatorPage::onBaseChanged(int index) {
    resetCeClearCycle();
    const int bases[] = {10, 16, 2, 8};
    m_engine.setBase(bases[index]);

    // "prec" is decimal-only; fallback to 64-bit in non-decimal bases.
    if (bases[index] != 10 && m_widthCombo->currentIndex() == 4)
        m_widthCombo->setCurrentIndex(3);

    updateDisplay();
}

void CalculatorPage::onWordWidthChanged(int index) {
    resetCeClearCycle();
    const int widths[] = {8, 16, 32, 64};
    const bool precMode = (index == 4);
    m_engine.setBigMode(precMode);
    if (!precMode)
        m_engine.setWordBits(widths[index]);
    updateDisplay();
}

void CalculatorPage::onDigitClicked() {
    auto* btn = qobject_cast<QPushButton*>(sender());
    pressDigit(btn->text());
}

void CalculatorPage::pressDigit(const QString& d) {
    resetCeClearCycle();
    m_engine.pressDigit(d);
    updateDisplay();
}

void CalculatorPage::onOperatorClicked() {
    auto* btn = qobject_cast<QPushButton*>(sender());
    QString op = btn->text();
    if (op == "÷") op = "/";
    if (op == "×") op = "*";
    pressOperator(op);
}

void CalculatorPage::pressOperator(const QString& op) {
    resetCeClearCycle();
    m_engine.pressOperator(op);
    updateDisplay();
}

void CalculatorPage::onEqualsClicked() {
    resetCeClearCycle();
    m_engine.equals();
    updateDisplay();
}

void CalculatorPage::onClearEntryOrAllClicked() {
    const bool hasVisibleInput = (m_engine.displayText() != "0") || !m_engine.expressionText().isEmpty();
    if (!m_ceEntryCleared && hasVisibleInput) {
        m_engine.clearEntry();
        m_ceEntryCleared = true;
    } else {
        m_engine.clearAll();
        m_ceEntryCleared = false;
    }
    updateDisplay();
}

void CalculatorPage::onClearClicked() {
    resetCeClearCycle();
    m_engine.clearAll();
    updateDisplay();
}

void CalculatorPage::onBackspaceClicked() {
    resetCeClearCycle();
    m_engine.backspace();
    updateDisplay();
}

void CalculatorPage::onNegateClicked() {
    resetCeClearCycle();
    m_engine.negate();
    updateDisplay();
}

void CalculatorPage::onPiClicked() {
    resetCeClearCycle();
    auto* btn = qobject_cast<QPushButton*>(sender());
    QString op = btn->objectName();

    if (op == "π") m_engine.setPi();
    else if (op == "e") m_engine.setEuler();

    updateDisplay();
}

void CalculatorPage::onBitwiseClicked() {
    resetCeClearCycle();
    auto* btn = qobject_cast<QPushButton*>(sender());
    QString op = btn->objectName();
    m_engine.applyBitwiseOrFunction(op);
    updateDisplay();
}

void CalculatorPage::resetCeClearCycle() {
    m_ceEntryCleared = false;
}

// ─── Float formatting ─────────────────────────────────────────────────────────
QString CalculatorPage::formatDouble(double val) {
    return CalculatorCore::formatDouble(val);
}

// ─── Theme ────────────────────────────────────────────────────────────────────
void CalculatorPage::applyTheme(bool dark) {
    const QString numS  = ThemeColors::calcNumButton(dark);
    const QString opS   = ThemeColors::calcOpButton(dark);
    const QString bitS  = ThemeColors::calcBitButton(dark);
    const QString funcS = ThemeColors::calcFuncButton(dark);
    const QString hexS  = ThemeColors::calcHexButton(dark);
    const QString eqS   = ThemeColors::calcEqButton(dark);
    const QString clrS  = ThemeColors::calcClearButton(dark);
    const QString secS  = ThemeColors::calcSecondFuncButton(dark);
    const QString dispS = ThemeColors::calcDisplayStyle(dark);
    const QString exprS = ThemeColors::calcExprStyle(dark);
    const QString hintS = ThemeColors::calcHintStyle(dark);

    // ── Apply ─────────────────────────────────────────────────────────────────
    m_display->setStyleSheet(dispS);
    m_exprLabel->setStyleSheet(exprS);
    m_hintLabel->setStyleSheet(hintS);

    for (auto* b : m_numBtns)    b->setStyleSheet(numS);
    for (auto* b : m_opBtns)     b->setStyleSheet(opS);
    for (auto* b : m_bitOpBtns)  b->setStyleSheet(bitS);
    for (auto* b : m_funcBtns)   b->setStyleSheet(funcS);
    for (auto* b : m_clearBtns)  b->setStyleSheet(clrS);
    for (auto* b : m_hexBtns)    b->setStyleSheet(hexS);
    if (m_eqBtn) m_eqBtn->setStyleSheet(eqS);
}

// ─── Keyboard support ────────────────────────────────────────────────────────
void CalculatorPage::keyPressEvent(QKeyEvent* event) {
    const int key = event->key();
    const Qt::KeyboardModifiers mod = event->modifiers();

    if (mod == Qt::ControlModifier && key == Qt::Key_C) {
        if (auto* clipboard = QGuiApplication::clipboard())
            clipboard->setText(m_engine.displayText());
        return;
    }

    if (mod == Qt::ControlModifier && key == Qt::Key_V) {
        auto* clipboard = QGuiApplication::clipboard();
        if (!clipboard)
            return;

        QString text = clipboard->text().trimmed().toUpper();
        if (text.isEmpty())
            return;

        text.remove(' ');
        text.remove('_');
        text.remove('\'');

        if (m_engine.base() == 16 && text.startsWith("0X")) text.remove(0, 2);
        if (m_engine.base() == 2  && text.startsWith("0B")) text.remove(0, 2);
        if (m_engine.base() == 8  && text.startsWith("0O")) text.remove(0, 2);

        bool negative = false;
        if (text.startsWith('-')) {
            negative = true;
            text.remove(0, 1);
        } else if (text.startsWith('+')) {
            text.remove(0, 1);
        }

        if (text.isEmpty())
            return;

        const int base = m_engine.base();
        bool hasValidDigit = false;
        for (const QChar ch : text) {
            if (ch.isDigit()) {
                const int v = ch.unicode() - '0';
                if ((base == 2 && v > 1) || (base == 8 && v > 7))
                    return;
                hasValidDigit = true;
                continue;
            }
            if (base == 16 && ch >= 'A' && ch <= 'F') {
                hasValidDigit = true;
                continue;
            }
            if ((ch == '.' || ch == ',') && base == 10)
                continue;
            return;
        }

        if (!hasValidDigit)
            return;

        resetCeClearCycle();
        m_engine.clearEntry();

        for (const QChar ch : text) {
            if (ch == ',') {
                m_engine.pressDigit(".");
            } else {
                m_engine.pressDigit(QString(ch));
            }
        }

        if (negative)
            m_engine.negate();

        updateDisplay();
        return;
    }


    // ── Decimal point (float mode) ────────────────────────────────────────────
    if (key == Qt::Key_Period || key == Qt::Key_Comma) {
        pressDigit(".");
        return;
    }

    if (key == Qt::Key_ParenLeft) {
        pressDigit("(");
        return;
    }
    if (key == Qt::Key_ParenRight) {
        pressDigit(")");
        return;
    }

    // ── Digits 0-9 ───────────────────────────────────────────────────────────
    const bool hasBlockedModifier = (mod & (Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier));
    if (key >= Qt::Key_0 && key <= Qt::Key_9 && !hasBlockedModifier) {
        QString d = QString::number(key - Qt::Key_0);
        // In non-decimal bases, check validity
        if (m_engine.base() == 2  && d.toInt() > 1)  { QWidget::keyPressEvent(event); return; }
        if (m_engine.base() == 8  && d.toInt() > 7)  { QWidget::keyPressEvent(event); return; }
        pressDigit(d);
        return;
    }
    // Numpad digits
    if (key >= Qt::Key_0 + 0x20 && key <= Qt::Key_0 + 0x29) { // Key_0 numpad offset varies
        // handled by numeric keys above via Qt::Key_0..9
    }

    // ── Hex letters A-F ──────────────────────────────────────────────────────
    if (m_engine.base() == 16 && (mod == Qt::NoModifier || mod == Qt::ShiftModifier)) {
        if (key >= Qt::Key_A && key <= Qt::Key_F) {
            pressDigit(QString(QChar('A' + (key - Qt::Key_A))));
            return;
        }
        // lowercase a-f is mapped to same keys with no modifier
    }

    // ── Operators ────────────────────────────────────────────────────────────
    switch (key) {
    case Qt::Key_Plus:       pressOperator("+"); return;
    case Qt::Key_Minus:      pressOperator("-"); return;
    case Qt::Key_Asterisk:   pressOperator("*"); return;
    case Qt::Key_Slash:      pressOperator("/"); return;
    case Qt::Key_Percent:    pressOperator("MOD"); return;

    // ── Equals / Enter ───────────────────────────────────────────────────────
    case Qt::Key_Return:
    case Qt::Key_Enter:
    case Qt::Key_Equal:      onEqualsClicked(); return;

    // ── Clear ────────────────────────────────────────────────────────────────
    case Qt::Key_Escape:
        if (m_engine.isClearState()) {
            if (QWidget* w = window())
                w->close();
            return;
        }
        onClearClicked();
        return;
    case Qt::Key_Delete:     onClearClicked(); return;
    case Qt::Key_Backspace:  onBackspaceClicked(); return;

    // ── Bitwise ops via keyboard shortcuts ───────────────────────────────────
    // & = AND, | = OR, ^ = XOR, ~ = NOT, < = LSL, > = LSR
    case Qt::Key_Ampersand:  // & → AND (first operand)
        m_engine.applyBitwiseOrFunction("AND");
        updateDisplay();
        return;
    case Qt::Key_Bar:        // | → OR
        m_engine.applyBitwiseOrFunction("OR");
        updateDisplay();
        return;
    case Qt::Key_AsciiCircum: // ^ → XOR
        m_engine.applyBitwiseOrFunction("XOR");
        updateDisplay();
        return;
    case Qt::Key_AsciiTilde: // ~ → NOT (immediate)
        m_engine.applyBitwiseOrFunction("NOT");
        updateDisplay();
        return;
    case Qt::Key_Less:       // < → LSL (shift left)
        m_engine.applyBitwiseOrFunction("LSL");
        updateDisplay();
        return;
    case Qt::Key_Greater:    // > → LSR (logical shift right)
        m_engine.applyBitwiseOrFunction("LSR");
        updateDisplay();
        return;
    case Qt::Key_N:          // N → NEG
        if (mod == Qt::NoModifier) { onNegateClicked(); return; }
        break;

    // ── Base switching shortcuts ──────────────────────────────────────────────
    // Ctrl+D = Decimal, Ctrl+H = Hex, Ctrl+B = Binary, Ctrl+O = Octal
    case Qt::Key_D:
        if (mod == Qt::ControlModifier) { m_baseCombo->setCurrentIndex(0); return; }
        break;
    case Qt::Key_H:
        if (mod == Qt::ControlModifier) { m_baseCombo->setCurrentIndex(1); return; }
        break;
    case Qt::Key_B:
        if (mod == Qt::ControlModifier) { m_baseCombo->setCurrentIndex(2); return; }
        break;
    case Qt::Key_O:
        if (mod == Qt::ControlModifier) { m_baseCombo->setCurrentIndex(3); return; }
        break;

    // ── Word width shortcuts ─────────────────────────────────────────────────
    // Ctrl+1..5 = 8/16/32/64-bit/Scientific
    case Qt::Key_1:
        if (mod == Qt::ControlModifier) { m_widthCombo->setCurrentIndex(0); return; }
        break;
    case Qt::Key_2:
        if (mod == Qt::ControlModifier) { m_widthCombo->setCurrentIndex(1); return; }
        break;
    case Qt::Key_3:
        if (mod == Qt::ControlModifier) { m_widthCombo->setCurrentIndex(2); return; }
        break;
    case Qt::Key_4:
        if (mod == Qt::ControlModifier) { m_widthCombo->setCurrentIndex(3); return; }
        break;
    case Qt::Key_5:
        if (mod == Qt::ControlModifier) { m_widthCombo->setCurrentIndex(4); return; }
        break;

    default: break;
    }

    QWidget::keyPressEvent(event);
}

