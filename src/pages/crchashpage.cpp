#include "crchashpage.h"

#include "core/crchash.h"
#include "ui/themecolors.h"

#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRegularExpression>
#include <QScrollArea>
#include <QSizePolicy>
#include <QVBoxLayout>

namespace {

enum class InputMode { Text, Hex, Decimal };

QByteArray bytesFromHex(const QString& raw, bool& ok) {
    QString cleaned = raw;
    cleaned.remove(QRegularExpression("0[xX]"));
    cleaned.remove(QRegularExpression("[\\s,;]"));

    if (cleaned.isEmpty()) {
        ok = true;
        return {};
    }
    if (cleaned.size() % 2 != 0 || !cleaned.contains(QRegularExpression("^[0-9A-Fa-f]+$"))) {
        ok = false;
        return {};
    }

    ok = true;
    return QByteArray::fromHex(cleaned.toUtf8());
}

QByteArray bytesFromDecimal(const QString& raw, bool& ok) {
    const QStringList tokens = raw.split(QRegularExpression("[\\s,;]+"), Qt::SkipEmptyParts);
    QByteArray result;
    result.reserve(tokens.size());
    for (const QString& token : tokens) {
        bool tokenOk = false;
        const int value = token.toInt(&tokenOk);
        if (!tokenOk || value < 0 || value > 255) {
            ok = false;
            return {};
        }
        result.append(static_cast<char>(value));
    }
    ok = true;
    return result;
}

} // namespace

CrcHashPage::CrcHashPage(QWidget* parent) : QWidget(parent) {
    setupUI();
    applyTheme(true);
    recalculate();
}

void CrcHashPage::setupUI() {
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
    root->setSpacing(8);
    root->setContentsMargins(12, 12, 12, 12);

    m_titleLabel = new QLabel("CRC and Hash Tools", this);
    root->addWidget(m_titleLabel);

    m_inputGroup = new QGroupBox("Input", this);
    auto* inGrid = new QGridLayout(m_inputGroup);
    inGrid->setHorizontalSpacing(8);
    inGrid->setVerticalSpacing(6);

    m_algoLabel = new QLabel("Algorithm:", this);
    m_algoCombo = new QComboBox(this);
    for (const auto& entry : Rheno::Core::algorithms()) {
        m_algoCombo->addItem(entry.name, static_cast<int>(entry.algorithm));
    }

    inGrid->addWidget(m_algoLabel, 0, 0);
    inGrid->addWidget(m_algoCombo, 0, 1);

    m_modeLabel = new QLabel("Input format:", this);
    m_modeCombo = new QComboBox(this);
    m_modeCombo->addItem("Text (UTF-8)", static_cast<int>(InputMode::Text));
    m_modeCombo->addItem("Hex", static_cast<int>(InputMode::Hex));
    m_modeCombo->addItem("Decimal", static_cast<int>(InputMode::Decimal));

    inGrid->addWidget(m_modeLabel, 1, 0);
    inGrid->addWidget(m_modeCombo, 1, 1);

    m_inputLabel = new QLabel("Input (UTF-8 text):", this);
    m_inputEdit = new QPlainTextEdit(this);
    m_inputEdit->setPlaceholderText("Enter text or payload here...");
    m_inputEdit->setMinimumHeight(140);

    inGrid->addWidget(m_inputLabel, 2, 0, 1, 2);
    inGrid->addWidget(m_inputEdit, 3, 0, 1, 2);

    root->addWidget(m_inputGroup);

    m_outputGroup = new QGroupBox("Output", this);
    auto* outGrid = new QGridLayout(m_outputGroup);
    outGrid->setHorizontalSpacing(8);
    outGrid->setVerticalSpacing(6);

    auto* outLabel = new QLabel("Checksum / Hash:", this);
    m_outputEdit = new QLineEdit(this);
    m_outputEdit->setReadOnly(true);
    m_outputEdit->setPlaceholderText("Result appears here");

    m_copyBtn = new QPushButton("Copy", this);

    outGrid->addWidget(outLabel, 0, 0);
    outGrid->addWidget(m_outputEdit, 0, 1);
    outGrid->addWidget(m_copyBtn, 0, 2);
    outGrid->setColumnStretch(1, 1);

    m_statusLabel = new QLabel("Ready", this);
    m_statusLabel->setWordWrap(true);
    outGrid->addWidget(m_statusLabel, 1, 0, 1, 3);

    root->addWidget(m_outputGroup);

    m_infoGroup = new QGroupBox("Info", this);
    auto* infoLayout = new QVBoxLayout(m_infoGroup);
    infoLayout->setContentsMargins(8, 8, 8, 8);
    infoLayout->setSpacing(4);
    m_formulaLabel = new QLabel(this);
    m_formulaLabel->setWordWrap(true);
    m_formulaLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    infoLayout->addWidget(m_formulaLabel);
    root->addWidget(m_infoGroup);

    root->addStretch();

    connect(m_algoCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CrcHashPage::recalculate);
    connect(m_modeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CrcHashPage::recalculate);
    connect(m_inputEdit, &QPlainTextEdit::textChanged, this, &CrcHashPage::recalculate);
    connect(m_copyBtn, &QPushButton::clicked, this, &CrcHashPage::copyResult);

    scroll->setWidget(content);
    outer->addWidget(scroll);
}

void CrcHashPage::recalculate() {
    const auto mode = static_cast<InputMode>(m_modeCombo->currentData().toInt());
    const QString rawInput = m_inputEdit->toPlainText();

    QByteArray data;
    bool ok = true;
    switch (mode) {
    case InputMode::Text:
        m_inputLabel->setText("Input (UTF-8 text):");
        m_inputEdit->setPlaceholderText("Enter text or payload here...");
        data = rawInput.toUtf8();
        break;
    case InputMode::Hex:
        m_inputLabel->setText("Input (Hex, e.g. 4A 3F 01 or 0x4A,0x3F):");
        m_inputEdit->setPlaceholderText("Enter hex bytes here...");
        data = bytesFromHex(rawInput, ok);
        break;
    case InputMode::Decimal:
        m_inputLabel->setText("Input (Decimal bytes 0-255, e.g. 12 253 7):");
        m_inputEdit->setPlaceholderText("Enter decimal byte values here...");
        data = bytesFromDecimal(rawInput, ok);
        break;
    }

    if (!ok) {
        m_outputEdit->clear();
        m_statusLabel->setText("Invalid input for selected format");
        return;
    }

    const auto alg = static_cast<Rheno::Core::Algorithm>(m_algoCombo->currentData().toInt());
    const Rheno::Core::ComputeResult result = Rheno::Core::compute(alg, data);

    m_outputEdit->setText(result.value);
    m_statusLabel->setText(QString("%1 bytes processed").arg(data.size()));
    m_formulaLabel->setText(result.formula);
}

int CrcHashPage::algorithmIndex() const {
    return m_algoCombo->currentIndex();
}

void CrcHashPage::setAlgorithmIndex(int index) {
    if (index >= 0 && index < m_algoCombo->count())
        m_algoCombo->setCurrentIndex(index);
}

int CrcHashPage::inputFormatIndex() const {
    return m_modeCombo->currentIndex();
}

void CrcHashPage::setInputFormatIndex(int index) {
    if (index >= 0 && index < m_modeCombo->count())
        m_modeCombo->setCurrentIndex(index);
}

void CrcHashPage::copyResult() {
    m_outputEdit->selectAll();
    m_outputEdit->copy();
    m_outputEdit->deselect();
    m_statusLabel->setText("Result copied to clipboard");
}

void CrcHashPage::applyTheme(bool dark) {
    const QString grpS = Rheno::UI::unitGroupStyle(dark);
    const QString fldS = Rheno::UI::unitFieldStyle(dark);
    const QString resS = Rheno::UI::unitResultStyle(dark);
    const QString ttlS = Rheno::UI::unitTitleStyle(dark);
    const QString frmS = Rheno::UI::unitFormulaStyle(dark);

    m_titleLabel->setStyleSheet(ttlS);
    m_inputGroup->setStyleSheet(grpS);
    m_outputGroup->setStyleSheet(grpS);
    m_infoGroup->setStyleSheet(grpS);

    m_inputEdit->setStyleSheet(fldS);
    m_outputEdit->setStyleSheet(resS);
    m_statusLabel->setStyleSheet(frmS);
    m_formulaLabel->setStyleSheet(frmS);
}

