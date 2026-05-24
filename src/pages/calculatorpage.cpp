#include "calculatorpage.h"
#include "ui/themecolors.h"
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFont>
#include <QFrame>
#include <QToolTip>
#include <cmath>
#include <climits>
#include <limits>

CalculatorPage::CalculatorPage(QWidget* parent) : QWidget(parent) {
    setupUI();
}

QPushButton* CalculatorPage::makeBtn(const QString& text, const QString& style) {
    auto* btn = new QPushButton(text, this);
    btn->setMinimumSize(58, 44);
    btn->setStyleSheet(style.isEmpty() ? ThemeColors::calcNumButton(true) : style);
    return btn;
}

void CalculatorPage::setupUI() {
    auto* root = new QVBoxLayout(this);
    root->setSpacing(8);
    root->setContentsMargins(12, 12, 12, 12);

    // Top controls
    auto* topRow = new QHBoxLayout();
    m_baseLabel = new QLabel("Base:", this);
    m_baseLabel->setStyleSheet("font-size:13px;");
    m_baseCombo = new QComboBox(this);
    m_baseCombo->addItems({"Decimal (10)", "Hexadecimal (16)", "Binary (2)", "Octal (8)"});
    m_baseCombo->setMinimumWidth(160);

    m_wordLabel = new QLabel("Word:", this);
    m_wordLabel->setStyleSheet("font-size:13px;");
    m_widthCombo = new QComboBox(this);
    m_widthCombo->addItems({"8-bit", "16-bit", "32-bit", "64-bit"});
    m_widthCombo->setCurrentIndex(2);

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
    m_display->setMinimumHeight(56);
    root->addWidget(m_display);

    // Keyboard shortcut hint bar
    m_hintLabel = new QLabel(
        "⌨  0–9 / A–F  |  + - * /  |  % MOD  |  & AND  |  | OR  |  ^ XOR  |  ~ NOT  |  < LSL  |  > LSR  |  "
        ". / ,  Dezimal  |  Enter =  |  Esc AC  |  ⌫ BS  |  Ctrl+D/H/B/O: Base  |  Ctrl+1–4: Word width  |  Shift+◀▶: Reiter",
        this);
    m_hintLabel->setWordWrap(true);
    root->addWidget(m_hintLabel);

    // Button grid
    auto* grid = new QGridLayout();
    grid->setSpacing(6);

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

    auto* negBtn = makeBtn("+/-", ThemeColors::calcFuncButton(true));
    m_funcBtns << negBtn;
    connect(negBtn, &QPushButton::clicked, this, &CalculatorPage::onNegateClicked);
    grid->addWidget(negBtn, 1, 5);

    auto* clrBtn = makeBtn("CE", ThemeColors::calcClearButton(true));
    m_clearBtns << clrBtn;
    connect(clrBtn, &QPushButton::clicked, this, [this]{ m_current=0; m_newInput=true; updateDisplay(); });
    grid->addWidget(clrBtn, 1, 6);

    auto* clearAllBtn = makeBtn("AC", ThemeColors::calcClearButton(true));
    m_clearBtns << clearAllBtn;
    connect(clearAllBtn, &QPushButton::clicked, this, &CalculatorPage::onClearClicked);
    grid->addWidget(clearAllBtn, 2, 4);

    auto* powBtn = makeBtn("x²", ThemeColors::calcFuncButton(true));
    powBtn->setObjectName("SQ");
    m_funcBtns << powBtn;
    connect(powBtn, &QPushButton::clicked, this, &CalculatorPage::onBitwiseClicked);
    grid->addWidget(powBtn, 2, 5);

    auto* sqrtBtn = makeBtn("√x", ThemeColors::calcFuncButton(true));
    sqrtBtn->setObjectName("SQRT");
    m_funcBtns << sqrtBtn;
    connect(sqrtBtn, &QPushButton::clicked, this, &CalculatorPage::onBitwiseClicked);
    grid->addWidget(sqrtBtn, 2, 6);

    // Digits + operators (rows 3-6)
    struct BtnDef { QString label; int row, col; QString style; };
    QList<BtnDef> defs = {
        {"7",3,0,ThemeColors::calcNumButton(true)},{"8",3,1,ThemeColors::calcNumButton(true)},{"9",3,2,ThemeColors::calcNumButton(true)},{"÷",3,3,ThemeColors::calcOpButton(true)},{"(",3,4,ThemeColors::calcFuncButton(true)},{")",3,5,ThemeColors::calcFuncButton(true)},{"<<",3,6,ThemeColors::calcBitButton(true)},
        {"4",4,0,ThemeColors::calcNumButton(true)},{"5",4,1,ThemeColors::calcNumButton(true)},{"6",4,2,ThemeColors::calcNumButton(true)},{"×",4,3,ThemeColors::calcOpButton(true)},{"1/x",4,4,ThemeColors::calcFuncButton(true)},{"abs",4,5,ThemeColors::calcFuncButton(true)},{">>",4,6,ThemeColors::calcBitButton(true)},
        {"1",5,0,ThemeColors::calcNumButton(true)},{"2",5,1,ThemeColors::calcNumButton(true)},{"3",5,2,ThemeColors::calcNumButton(true)},{"-",5,3,ThemeColors::calcOpButton(true)},{"MS",5,4,ThemeColors::calcFuncButton(true)},{"MR",5,5,ThemeColors::calcFuncButton(true)},{"MC",5,6,ThemeColors::calcFuncButton(true)},
        {"0",6,0,ThemeColors::calcNumButton(true)},{"00",6,1,ThemeColors::calcNumButton(true)},{".",6,2,ThemeColors::calcNumButton(true)},{"+",6,3,ThemeColors::calcOpButton(true)},{"=",6,4,ThemeColors::calcEqButton(true)},
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
        } else if (d.label == "1/x" || d.label == "abs" || d.label == ">>" || d.label == "<<") {
            b->setObjectName(d.label);
            m_funcBtns << b;
            connect(b, &QPushButton::clicked, this, &CalculatorPage::onBitwiseClicked);
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

    // Initially disable hex letters
    for (int i = 0; i < 6; ++i) m_hexBtns[i]->setEnabled(false);

    // Enable keyboard input
    setFocusPolicy(Qt::StrongFocus);
}

long long CalculatorPage::maskToWidth(long long val) {
    if (m_wordBits == 64) return val;
    long long mask = (1LL << m_wordBits) - 1;
    return val & mask;
}

QString CalculatorPage::toBaseString(long long val) {
    long long masked = maskToWidth(val);
    if (m_base == 16) return QString::number((unsigned long long)masked, 16).toUpper();
    if (m_base == 2)  return QString::number((unsigned long long)masked, 2);
    if (m_base == 8)  return QString::number((unsigned long long)masked, 8);
    return QString::number(masked);
}

long long CalculatorPage::fromBaseString(const QString& s) {
    bool ok;
    long long v = s.toLongLong(&ok, m_base);
    return ok ? v : 0;
}

void CalculatorPage::updateDisplay() {
    if (m_floatMode) {
        // While the user is actively typing (m_inputString is being built), show the raw string.
        // Otherwise format the stored double nicely.
        if (!m_inputString.isEmpty()) {
            m_display->setText(m_inputString);
        } else {
            m_display->setText(formatDouble(m_currentDouble));
        }
    } else {
        m_display->setText(toBaseString(m_current));
    }
    bool hexMode = (m_base == 16);
    for (int i = 0; i < 6; ++i) m_hexBtns[i]->setEnabled(hexMode);
}

void CalculatorPage::onBaseChanged(int index) {
    const int bases[] = {10, 16, 2, 8};
    m_base = bases[index];
    // Leaving decimal mode: drop float state and convert to integer
    if (m_base != 10 && m_floatMode) {
        m_floatMode = false;
        m_inputString.clear();
        m_current = static_cast<long long>(m_currentDouble);
        m_currentDouble = 0.0;
    }
    m_newInput = true;
    updateDisplay();
}

void CalculatorPage::onWordWidthChanged(int index) {
    const int widths[] = {8, 16, 32, 64};
    m_wordBits = widths[index];
    m_current = maskToWidth(m_current);
    updateDisplay();
}

void CalculatorPage::onDigitClicked() {
    auto* btn = qobject_cast<QPushButton*>(sender());
    pressDigit(btn->text());
}

void CalculatorPage::pressDigit(const QString& d) {
    // ── Decimal point ──────────────────────────────────────────────────────────
    if (d == ".") {
        if (m_base != 10) return;
        if (m_floatMode && m_inputString.contains('.')) return; // already has a decimal point
        m_floatMode = true;
        if (m_newInput) {
            // No number entered yet → start with "0."
            m_inputString   = "0.";
            m_currentDouble = 0.0;
            m_newInput = false;
        } else if (m_inputString.isEmpty()) {
            // User typed digits in integer mode (m_inputString unused until now) → preserve them
            m_inputString   = QString::number(m_current) + ".";
            m_currentDouble = static_cast<double>(m_current);
        } else {
            // Already in float-mode input (digits after operator) → just append dot
            m_inputString  += ".";
            m_currentDouble = m_inputString.toDouble();
        }
        updateDisplay();
        return;
    }

    // ── Float mode: append digit to the input string ───────────────────────────
    if (m_floatMode) {
        if (m_newInput) {
            // Start a brand-new number but stay in float mode (operator was already pressed)
            m_inputString  = d;
            m_currentDouble = d.toDouble();
            m_newInput = false;
            updateDisplay();
            return;
        } else {
            m_inputString  += d;
            m_currentDouble = m_inputString.toDouble();
            updateDisplay();
            return;
        }
    }

    // ── Integer digit input ────────────────────────────────────────────────────
    if (m_newInput) { m_current = 0; m_newInput = false; }
    QString cur = toBaseString(m_current);
    if (cur == "0") cur = d;
    else cur += d;
    m_current = maskToWidth(fromBaseString(cur));
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
    if (!m_pendingOp.isEmpty()) {
        onEqualsClicked();
    }
    // In decimal mode, always use double arithmetic so that e.g. 7/3 = 2.333...
    if (m_base == 10 && !m_floatMode) {
        m_floatMode = true;
        m_currentDouble = static_cast<double>(m_current);
        m_inputString.clear();
    }
    if (m_floatMode) {
        m_accumulatorDouble = m_currentDouble;
        m_inputString.clear();
        m_pendingOp = op;
        m_exprLabel->setText(formatDouble(m_accumulatorDouble) + " " + op);
    } else {
        m_accumulator = m_current;
        m_pendingOp   = op;
        m_exprLabel->setText(toBaseString(m_accumulator) + " " + op);
    }
    m_newInput = true;
}

static long long memory = 0;

void CalculatorPage::onEqualsClicked() {
    if (m_pendingOp.isEmpty()) return;

    if (m_floatMode) {
        double a = m_accumulatorDouble, b = m_currentDouble;
        double res = a;
        if      (m_pendingOp == "+")   res = a + b;
        else if (m_pendingOp == "-")   res = a - b;
        else if (m_pendingOp == "*")   res = a * b;
        else if (m_pendingOp == "/")   res = (b != 0.0) ? a / b : std::numeric_limits<double>::infinity();
        else if (m_pendingOp == "MOD") res = std::fmod(a, b);
        m_exprLabel->setText(formatDouble(a) + " " + m_pendingOp + " " + formatDouble(b) + " =");
        m_currentDouble = res;
        m_inputString.clear();
        m_pendingOp.clear();
        m_newInput = true;
        updateDisplay();
        return;
    }

    long long a = m_accumulator, b = m_current;
    long long res = a;
    if      (m_pendingOp == "+")   res = a + b;
    else if (m_pendingOp == "-")   res = a - b;
    else if (m_pendingOp == "*")   res = a * b;
    else if (m_pendingOp == "/")   { res = (b != 0) ? a / b : 0; }
    else if (m_pendingOp == "MOD") { res = (b != 0) ? a % b : 0; }
    m_exprLabel->setText(toBaseString(a) + " " + m_pendingOp + " " + toBaseString(b) + " =");
    m_current = maskToWidth(res);
    m_pendingOp.clear();
    m_newInput = true;
    updateDisplay();
}

void CalculatorPage::onClearClicked() {
    m_current = 0; m_accumulator = 0; m_pendingOp.clear();
    m_currentDouble = 0.0; m_accumulatorDouble = 0.0;
    m_floatMode = false; m_inputString.clear();
    m_newInput = true; m_exprLabel->clear();
    updateDisplay();
}

void CalculatorPage::onBackspaceClicked() {
    if (m_floatMode) {
        if (m_inputString.isEmpty()) return;
        m_inputString.chop(1);
        // If we removed the decimal point or are left with just sign/empty → leave float mode
        if (m_inputString.isEmpty() || m_inputString == "-" || !m_inputString.contains('.')) {
            m_floatMode = false;
            m_current = m_inputString.isEmpty() ? 0 : static_cast<long long>(m_inputString.toDouble());
            m_inputString.clear();
        } else {
            m_currentDouble = m_inputString.toDouble();
        }
        updateDisplay();
        return;
    }
    QString s = toBaseString(m_current);
    if (s.length() > 1) s.chop(1); else s = "0";
    m_current = fromBaseString(s);
    updateDisplay();
}

void CalculatorPage::onNegateClicked() {
    if (m_floatMode) {
        m_currentDouble = -m_currentDouble;
        m_inputString.clear(); // will be reformatted by updateDisplay
    } else {
        m_current = maskToWidth(-m_current);
    }
    updateDisplay();
}

void CalculatorPage::onBitwiseClicked() {
    auto* btn = qobject_cast<QPushButton*>(sender());
    QString op = btn->objectName();
    long long a = m_accumulator, b = m_current;
    long long res = m_current;

    if (op == "AND") { m_accumulator = b; m_pendingOp = "AND"; m_exprLabel->setText(toBaseString(b) + " AND"); m_newInput=true; return; }
    if (op == "OR")  { m_accumulator = b; m_pendingOp = "OR";  m_exprLabel->setText(toBaseString(b) + " OR");  m_newInput=true; return; }
    if (op == "XOR") { m_accumulator = b; m_pendingOp = "XOR"; m_exprLabel->setText(toBaseString(b) + " XOR"); m_newInput=true; return; }

    if (!m_pendingOp.isEmpty() && (op == "AND" || op == "OR" || op == "XOR")) {
        if (m_pendingOp == "AND") res = a & b;
        if (m_pendingOp == "OR")  res = a | b;
        if (m_pendingOp == "XOR") res = a ^ b;
        m_pendingOp.clear(); m_newInput = true;
        m_current = maskToWidth(res); updateDisplay(); return;
    }

    // ── Float-aware unary ops ────────────────────────────────────────────────
    if (op == "SQ") {
        if (m_floatMode) { m_currentDouble *= m_currentDouble; m_inputString.clear(); updateDisplay(); return; }
        res = maskToWidth(b * b);
    }
    else if (op == "SQRT") {
        if (m_floatMode) {
            m_currentDouble = (m_currentDouble >= 0.0) ? std::sqrt(m_currentDouble) : std::numeric_limits<double>::quiet_NaN();
            m_inputString.clear(); updateDisplay(); return;
        }
        res = (b >= 0) ? (long long)std::sqrt((double)b) : 0;
    }
    else if (op == "abs") {
        if (m_floatMode) { m_currentDouble = std::fabs(m_currentDouble); m_inputString.clear(); updateDisplay(); return; }
        res = std::abs(b);
    }
    else if (op == "1/x") {
        if (m_floatMode) {
            m_currentDouble = (m_currentDouble != 0.0) ? 1.0 / m_currentDouble : std::numeric_limits<double>::infinity();
            m_exprLabel->setText("1 / " + formatDouble(m_currentDouble) + " =");
            m_inputString.clear(); updateDisplay(); return;
        }
        m_exprLabel->setText("1 / " + toBaseString(b) + " = ");
        if (b) {
            // Switch to float mode for the result
            m_floatMode = true;
            m_currentDouble = 1.0 / static_cast<double>(b);
            m_inputString.clear();
            m_newInput = true;
            updateDisplay();
        }
        return;
    }
    else if (op == "NOT")  res = maskToWidth(~b);
    else if (op == "LSL")  res = maskToWidth(b << 1);
    else if (op == "LSR")  res = maskToWidth((long long)((unsigned long long)b >> 1));
    else if (op == "<<")   res = maskToWidth(b << 1);
    else if (op == ">>")   res = maskToWidth((long long)((unsigned long long)b >> 1));
    else if (op == "ROL") {
        unsigned long long ub = (unsigned long long)maskToWidth(b);
        res = maskToWidth((long long)((ub << 1) | (ub >> (m_wordBits - 1))));
    }
    else if (op == "ROR") {
        unsigned long long ub = (unsigned long long)maskToWidth(b);
        res = maskToWidth((long long)((ub >> 1) | (ub << (m_wordBits - 1))));
    }
    else if (op == "MS") { memory = b; m_exprLabel->setText("M← " + toBaseString(b)); return; }
    else if (op == "MR") { res = memory; m_exprLabel->setText("M→ " + toBaseString(memory)); }
    else if (op == "MC") { memory = 0; m_exprLabel->setText("M cleared"); return; }

    m_current = res; m_newInput = true;
    updateDisplay();
}

// ─── Float formatting ─────────────────────────────────────────────────────────
QString CalculatorPage::formatDouble(double val) {
    if (std::isinf(val)) return val > 0 ? "∞" : "-∞";
    if (std::isnan(val)) return "NaN";
    return QString::number(val, 'g', 12);
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


    // ── Decimal point (float mode) ────────────────────────────────────────────
    if (key == Qt::Key_Period || key == Qt::Key_Comma) {
        pressDigit(".");
        return;
    }

    // ── Digits 0-9 ───────────────────────────────────────────────────────────
    if (key >= Qt::Key_0 && key <= Qt::Key_9) {
        QString d = QString::number(key - Qt::Key_0);
        // In non-decimal bases, check validity
        if (m_base == 2  && d.toInt() > 1)  { QWidget::keyPressEvent(event); return; }
        if (m_base == 8  && d.toInt() > 7)  { QWidget::keyPressEvent(event); return; }
        pressDigit(d);
        return;
    }
    // Numpad digits
    if (key >= Qt::Key_0 + 0x20 && key <= Qt::Key_0 + 0x29) { // Key_0 numpad offset varies
        // handled by numeric keys above via Qt::Key_0..9
    }

    // ── Hex letters A-F ──────────────────────────────────────────────────────
    if (m_base == 16) {
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
    case Qt::Key_Escape:     onClearClicked(); return;
    case Qt::Key_Delete:     onClearClicked(); return;
    case Qt::Key_Backspace:  onBackspaceClicked(); return;

    // ── Bitwise ops via keyboard shortcuts ───────────────────────────────────
    // & = AND, | = OR, ^ = XOR, ~ = NOT, < = LSL, > = LSR
    case Qt::Key_Ampersand:  // & → AND (first operand)
        m_accumulator = m_current;
        m_pendingOp = "AND";
        m_exprLabel->setText(toBaseString(m_accumulator) + " AND");
        m_newInput = true;
        return;
    case Qt::Key_Bar:        // | → OR
        m_accumulator = m_current;
        m_pendingOp = "OR";
        m_exprLabel->setText(toBaseString(m_accumulator) + " OR");
        m_newInput = true;
        return;
    case Qt::Key_AsciiCircum: // ^ → XOR
        m_accumulator = m_current;
        m_pendingOp = "XOR";
        m_exprLabel->setText(toBaseString(m_accumulator) + " XOR");
        m_newInput = true;
        return;
    case Qt::Key_AsciiTilde: // ~ → NOT (immediate)
        m_current = maskToWidth(~m_current);
        m_newInput = true;
        updateDisplay();
        return;
    case Qt::Key_Less:       // < → LSL (shift left)
        m_current = maskToWidth(m_current << 1);
        m_newInput = true;
        updateDisplay();
        return;
    case Qt::Key_Greater:    // > → LSR (logical shift right)
        m_current = maskToWidth((long long)((unsigned long long)m_current >> 1));
        m_newInput = true;
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
    // Ctrl+1..4 = 8/16/32/64-bit
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

    default: break;
    }

    QWidget::keyPressEvent(event);
}

