#include "calculatorpageuibuilder.h"

#include "../calculatorpage.h"

#include <QFont>
#include <QGridLayout>
#include <QVBoxLayout>

namespace {

QPushButton* makeBtn(CalculatorPage* page, const QString& text) {
    auto* btn = new QPushButton(text, page);
    btn->setMinimumSize(34, 28);
    btn->setFocusPolicy(Qt::NoFocus);
    return btn;
}

} // namespace

namespace Rheno::Ui {

void buildCalculatorPageUi(CalculatorPage* page) {
    auto* root = new QVBoxLayout(page);
    root->setSizeConstraint(QLayout::SetNoConstraint);
    root->setSpacing(6);
    root->setContentsMargins(8, 8, 8, 8);

    auto* topRow = new QHBoxLayout();
    page->m_baseLabel = new QLabel("Base:", page);
    page->m_baseCombo = new QComboBox(page);
    page->m_baseCombo->addItems({"Dec", "Hex", "Bin", "Oct"});
    page->m_baseCombo->setMinimumWidth(90);

    page->m_wordLabel = new QLabel("Mode:", page);
    page->m_widthCombo = new QComboBox(page);
    page->m_widthCombo->addItems({"8-bit", "16-bit", "32-bit", "64-bit", "Scient"});
    page->m_widthCombo->setCurrentIndex(4);

    topRow->addWidget(page->m_baseLabel);
    topRow->addWidget(page->m_baseCombo);
    topRow->addSpacing(16);
    topRow->addWidget(page->m_wordLabel);
    topRow->addWidget(page->m_widthCombo);
    topRow->addStretch();
    root->addLayout(topRow);

    page->m_baseCombo->installEventFilter(page);
    page->m_widthCombo->installEventFilter(page);

    page->m_exprLabel = new QLabel("", page);
    page->m_exprLabel->setAlignment(Qt::AlignRight);
    root->addWidget(page->m_exprLabel);

    page->m_display = new QLineEdit("0", page);
    page->m_display->setAlignment(Qt::AlignRight);
    page->m_display->setReadOnly(true);
    page->m_display->setFont(QFont("Consolas,Courier New,monospace", 20));
    page->m_display->setMinimumHeight(34);
    root->addWidget(page->m_display);

#if defined(Q_OS_MACOS)
    page->m_hintLabel = new QLabel(
        "Opt+D/X/B/O: Base  |  Opt+1–5: Mode  |  Opt+◀ ▶: Tab \n% MOD  |  & AND  |  | OR  |  ^ XOR  |  ~ NOT\n< LSL  |  > LSR  |  Esc AC/Close",
        page);
#else
    page->m_hintLabel = new QLabel(
        "% MOD  |  & AND  |  | OR  |  ^ XOR |  ~ NOT  |\n< LSL  |  > LSR  |  Enter =  |  Esc AC/Close  |  ⌫ BS  |\nAlt+D/X/B/O: Base  |  Alt+1–5: Mode  |  Alt+◀ ▶: Tab",
        page);
#endif

    page->m_hintLabel->setWordWrap(true);
    root->addWidget(page->m_hintLabel);

    auto* grid = new QGridLayout();
    grid->setSpacing(4);

    const char* hexLetters[] = {"A", "B", "C", "D", "E", "F"};
    for (int i = 0; i < 6; ++i) {
        page->m_hexBtns[i] = makeBtn(page, hexLetters[i]);
        QObject::connect(page->m_hexBtns[i], &QPushButton::clicked, page, &CalculatorPage::onDigitClicked);
        grid->addWidget(page->m_hexBtns[i], 0, i);
    }

    auto* bsBtn = makeBtn(page, "⌫");
    page->m_funcBtns << bsBtn;
    QObject::connect(bsBtn, &QPushButton::clicked, page, &CalculatorPage::onBackspaceClicked);
    grid->addWidget(bsBtn, 0, 6);

    const char* bitOps[] = {"AND", "OR", "XOR", "NOT", "LSL", "LSR", "ROL", "ROR"};
    for (int i = 0; i < 4; ++i) {
        auto* b = makeBtn(page, bitOps[i]);
        b->setObjectName(bitOps[i]);
        page->m_bitOpBtns << b;
        QObject::connect(b, &QPushButton::clicked, page, &CalculatorPage::onBitwiseClicked);
        grid->addWidget(b, 1, i);
    }
    for (int i = 4; i < 8; ++i) {
        auto* b = makeBtn(page, bitOps[i]);
        b->setObjectName(bitOps[i]);
        page->m_bitOpBtns << b;
        QObject::connect(b, &QPushButton::clicked, page, &CalculatorPage::onBitwiseClicked);
        grid->addWidget(b, 2, i - 4);
    }

    auto* modBtn = makeBtn(page, "MOD");
    modBtn->setObjectName("MOD");
    page->m_funcBtns << modBtn;
    QObject::connect(modBtn, &QPushButton::clicked, page, &CalculatorPage::onOperatorClicked);
    grid->addWidget(modBtn, 1, 4);

    auto* invBtn = makeBtn(page, "1/x");
    invBtn->setObjectName("1/x");
    page->m_funcBtns << invBtn;
    QObject::connect(invBtn, &QPushButton::clicked, page, &CalculatorPage::onBitwiseClicked);
    grid->addWidget(invBtn, 1, 5);

    auto* clrBtn = makeBtn(page, "CE");
    page->m_clearBtns << clrBtn;
    QObject::connect(clrBtn, &QPushButton::clicked, page, &CalculatorPage::onClearEntryOrAllClicked);
    grid->addWidget(clrBtn, 1, 6);

    page->m_sqBtn = makeBtn(page, "x²");
    page->m_sqBtn->setObjectName("SQ");
    page->m_funcBtns << page->m_sqBtn;
    QObject::connect(page->m_sqBtn, &QPushButton::clicked, page, &CalculatorPage::onBitwiseClicked);
    grid->addWidget(page->m_sqBtn, 3, 5);

    page->m_sqrtBtn = makeBtn(page, "√x");
    page->m_sqrtBtn->setObjectName("SQRT");
    page->m_funcBtns << page->m_sqrtBtn;
    QObject::connect(page->m_sqrtBtn, &QPushButton::clicked, page, &CalculatorPage::onBitwiseClicked);
    grid->addWidget(page->m_sqrtBtn, 3, 6);

    page->m_secondFuncBtn = makeBtn(page, "2nd");
    page->m_clearBtns << page->m_secondFuncBtn;
    QObject::connect(page->m_secondFuncBtn, &QPushButton::clicked, page, &CalculatorPage::onSecondFuncToggled);
    grid->addWidget(page->m_secondFuncBtn, 2, 6);

    enum BtnType { Num, Op, Func, Bit, Eq };
    struct BtnDef { QString label; int row; int col; BtnType type; };
    QList<BtnDef> defs = {
        {"7", 3, 0, Num}, {"8", 3, 1, Num}, {"9", 3, 2, Num},
        {"÷", 3, 3, Op},
        {"(", 2, 4, Func}, {")", 2, 5, Func},
        {"π", 3, 4, Bit},
        {"4", 4, 0, Num}, {"5", 4, 1, Num}, {"6", 4, 2, Num},
        {"×", 4, 3, Op},
        {"e", 4, 4, Func}, {"log", 4, 5, Func}, {"ln", 4, 6, Func},
        {"1", 5, 0, Num}, {"2", 5, 1, Num}, {"3", 5, 2, Num},
        {"-", 5, 3, Op},
        {"MS", 5, 4, Func}, {"MR", 5, 5, Func}, {"MC", 5, 6, Func},
        {"+/-", 6, 0, Func},
        {"0", 6, 1, Num}, {".", 6, 2, Num},
        {"+", 6, 3, Op},
        {"=", 6, 4, Eq},
    };

    for (const auto& d : defs) {
        auto* b = makeBtn(page, d.label);
        if (d.label == "=") {
            b->setObjectName("=");
            page->m_eqBtn = b;
            grid->addWidget(b, d.row, d.col, 1, 3);
            QObject::connect(b, &QPushButton::clicked, page, &CalculatorPage::onEqualsClicked);
        } else if (d.label == "÷" || d.label == "×" || d.label == "+" || d.label == "-" || d.label == "MOD") {
            b->setObjectName(d.label);
            page->m_opBtns << b;
            QObject::connect(b, &QPushButton::clicked, page, &CalculatorPage::onOperatorClicked);
            grid->addWidget(b, d.row, d.col);
        } else if (d.label == "π" || d.label == "e") {
            b->setObjectName(d.label);
            page->m_funcBtns << b;
            QObject::connect(b, &QPushButton::clicked, page, &CalculatorPage::onPiClicked);
            grid->addWidget(b, d.row, d.col);
            if (d.label == "π") page->m_piBtn = b;
            if (d.label == "e") page->m_eBtn = b;
        } else if (d.label == "1/x" || d.label == "log" || d.label == "ln") {
            b->setObjectName(d.label);
            page->m_funcBtns << b;
            QObject::connect(b, &QPushButton::clicked, page, &CalculatorPage::onBitwiseClicked);
            grid->addWidget(b, d.row, d.col);
            if (d.label == "log") page->m_logBtn = b;
            if (d.label == "ln") page->m_lnBtn = b;
        } else if (d.label == "+/-") {
            page->m_funcBtns << b;
            QObject::connect(b, &QPushButton::clicked, page, &CalculatorPage::onNegateClicked);
            grid->addWidget(b, d.row, d.col);
        } else if (d.label == "MS" || d.label == "MR" || d.label == "MC") {
            b->setObjectName(d.label);
            page->m_funcBtns << b;
            QObject::connect(b, &QPushButton::clicked, page, &CalculatorPage::onBitwiseClicked);
            grid->addWidget(b, d.row, d.col);
            if (d.label == "MS") page->m_msBtn = b;
            if (d.label == "MR") page->m_mrBtn = b;
            if (d.label == "MC") page->m_mcBtn = b;
        } else if (d.label == "(" || d.label == ")") {
            page->m_funcBtns << b;
            QObject::connect(b, &QPushButton::clicked, page, &CalculatorPage::onDigitClicked);
            grid->addWidget(b, d.row, d.col);
        } else {
            page->m_numBtns << b;
            QObject::connect(b, &QPushButton::clicked, page, &CalculatorPage::onDigitClicked);
            grid->addWidget(b, d.row, d.col);
        }
    }

    root->addLayout(grid);
    root->addStretch();

    QObject::connect(page->m_baseCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), page, &CalculatorPage::onBaseChanged);
    QObject::connect(page->m_widthCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), page, &CalculatorPage::onWordWidthChanged);

    for (auto* btn : page->m_hexBtns)
        btn->setEnabled(false);

    page->setFocusPolicy(Qt::StrongFocus);
}

} // namespace Rheno::Ui

