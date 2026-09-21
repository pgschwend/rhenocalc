#include "colorpage.h"
#include "core/color.h"
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QScrollArea>
#include <QSizePolicy>
#include <QClipboard>
#include <QApplication>
#include <QTimer>
#include <QPixmap>
#include <QMouseEvent>
#include <QPainter>
#include <QtMath>
#include <QIntValidator>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QSettings>
#include <QScreen>
#include <QGuiApplication>
#include <QKeyEvent>

// ─── ScreenPicker (internal helper) ──────────────────────────────────────────

class ScreenPicker : public QWidget {
    Q_OBJECT
public:
    explicit ScreenPicker(QWidget* = nullptr) : QWidget(nullptr) {
        setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
        setAttribute(Qt::WA_TranslucentBackground);
        setCursor(Qt::CrossCursor);
        QScreen* screen = QGuiApplication::primaryScreen();
        if (screen) {
            m_screenshot = screen->grabWindow(0);
            setGeometry(screen->geometry());
        }
        showFullScreen();
    }
signals:
    void colorPicked(QColor color);
protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.drawPixmap(0, 0, m_screenshot);
        p.fillRect(rect(), QColor(0, 0, 0, 40));
    }
    void mousePressEvent(QMouseEvent* event) override {
        if (event->button() == Qt::LeftButton && !m_screenshot.isNull()) {
            // event->pos() is in logical (device-independent) coordinates, but the
            // grabbed screenshot is stored at physical pixel resolution; on a scaled
            // (HiDPI/Retina) display these differ and must be converted, or the
            // sampled pixel doesn't match where the user actually clicked.
            const qreal dpr = m_screenshot.devicePixelRatio();
            const QPoint physicalPos = (QPointF(event->pos()) * dpr).toPoint();
            const QImage img = m_screenshot.toImage();
            if (img.rect().contains(physicalPos))
                emit colorPicked(img.pixelColor(physicalPos));
        }
        close(); deleteLater();
    }
    void keyPressEvent(QKeyEvent* event) override {
        if (event->key() == Qt::Key_Escape) { close(); deleteLater(); }
    }
    bool event(QEvent* e) override {
        // The overlay only closes on a click or Escape while it has focus; if the
        // user switches away (Alt-Tab, a system dialog stealing focus, ...) before
        // that, it would otherwise stay stuck full-screen and always-on-top.
        if (e->type() == QEvent::WindowDeactivate) {
            close(); deleteLater();
            return true;
        }
        return QWidget::event(e);
    }
private:
    QPixmap m_screenshot;
};
ColorPage::ColorPage(QWidget* parent) : QWidget(parent), m_currentColor("#3498db") { setupUI(); }
void ColorPage::setupUI() {
    setMinimumSize(0, 0);
    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    auto* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    auto* content = new QWidget;
    auto* mainLayout = new QVBoxLayout(content);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(16, 16, 16, 16);

    // Color Wheel
    m_colorWheel = new ColorWheel;
    m_colorWheel->setMinimumSize(150, 150);
    m_colorWheel->setMaximumSize(220, 220);
    mainLayout->addWidget(m_colorWheel, 0, Qt::AlignHCenter);

    m_preview = new QLabel;
    m_preview->setMinimumHeight(40);
    m_preview->setMaximumHeight(80);
    m_preview->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_preview);

    m_colorName = new QLabel;
    m_colorName->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_colorName);

    m_rgbFormatCombo = new QComboBox;
    m_rgbFormatCombo->addItem("Decimal");
    m_rgbFormatCombo->addItem("Hex");
    m_rgbFormatCombo->setFixedWidth(90);
    m_rgbFormatCombo->setToolTip("Display format for RGB values on this page");
    m_pickBtn = new QPushButton("Pick");
    m_pickBtn->setMinimumWidth(60);
    m_pickBtn->setToolTip("Pick a color from screen");
    m_saveHistoryBtn = new QPushButton("Save");
    m_saveHistoryBtn->setMinimumWidth(60);
    m_saveHistoryBtn->setToolTip("Save the current color to History");
    auto* controlsRow = new QHBoxLayout();
    controlsRow->addWidget(m_rgbFormatCombo);
    controlsRow->addWidget(m_pickBtn);
    controlsRow->addWidget(m_saveHistoryBtn);
    controlsRow->addStretch();
    mainLayout->addLayout(controlsRow);

    m_pickStatusLabel = new QLabel;
    m_pickStatusLabel->setStyleSheet("font-size:11px;color:#4caf50;");
    m_pickStatusLabel->setAlignment(Qt::AlignCenter);
    // Hidden (rather than just empty) so it doesn't reserve layout space when idle --
    // otherwise the gap between the controls row and History would be wider than the
    // gap between History and the HEX group below it.
    m_pickStatusLabel->setVisible(false);
    mainLayout->addWidget(m_pickStatusLabel);

    m_historyGroup = new QGroupBox("History");
    m_historyLayout = new QHBoxLayout(m_historyGroup);
    m_historyLayout->addStretch();
    m_historyGroup->setVisible(false);
    mainLayout->addWidget(m_historyGroup);
    loadHistory();
    rebuildHistorySwatches();

    auto* hexGroup = new QGroupBox("HEX");
    auto* hexLayout = new QGridLayout(hexGroup);
    m_hexEdit = new QLineEdit("3498DB");
    // RGB only (6 hex digits) -- alpha has its own "A" field below (and its own
    // RRGGBBAA row further down). Allowing 8 digits here was ambiguous: QColor
    // parses "#AARRGGBB" (alpha first), which silently produced a very different
    // color than users pasting a CSS-style "#RRGGBBAA".
    m_hexEdit->setMaxLength(6);
    m_hexEdit->setToolTip("RRGGBB (alpha is set via the RGB group's \"A\" field)");
    hexLayout->addWidget(new QLabel("# RGB:"), 0, 0);
    hexLayout->addWidget(m_hexEdit, 0, 1);

    m_hexAlphaEdit = new QLineEdit("3498DBFF");
    m_hexAlphaEdit->setMaxLength(8);
    m_hexAlphaEdit->setValidator(new QRegularExpressionValidator(QRegularExpression("^[0-9A-Fa-f]{0,8}$"), m_hexAlphaEdit));
    m_hexAlphaEdit->setToolTip("RRGGBBAA (CSS order, alpha last)");
    hexLayout->addWidget(new QLabel("# RGBA:"), 1, 0);
    hexLayout->addWidget(m_hexAlphaEdit, 1, 1);

    mainLayout->addWidget(hexGroup);

    auto mk = [](int max) {
        auto* e = new QLineEdit("0");
        e->setAlignment(Qt::AlignCenter);
        e->setValidator(new QIntValidator(0, max, e));
        return e;
    };
    auto lbl = [](const QString& t) { auto* l = new QLabel(t); l->setFixedWidth(l->fontMetrics().horizontalAdvance(t) + 4); return l; };
    auto* rgbGroup = new QGroupBox("RGB");
    auto* rgbL = new QGridLayout(rgbGroup);
    m_redEdit = mk(255); m_greenEdit = mk(255); m_blueEdit = mk(255); m_alphaEdit = mk(255);
    m_alphaEdit->setText("255");
    rgbL->addWidget(lbl("R"),0,0); rgbL->addWidget(m_redEdit,0,1);
    rgbL->addWidget(lbl("G"),0,2); rgbL->addWidget(m_greenEdit,0,3);
    rgbL->addWidget(lbl("B"),1,0); rgbL->addWidget(m_blueEdit,1,1);
    rgbL->addWidget(lbl("A"),1,2); rgbL->addWidget(m_alphaEdit,1,3);
    mainLayout->addWidget(rgbGroup);
    auto* hslGroup = new QGroupBox("HSL");
    auto* hslL = new QGridLayout(hslGroup);
    m_hueEdit = mk(359); m_satEdit = mk(255); m_lightEdit = mk(255);
    hslL->addWidget(lbl("H"),0,0); hslL->addWidget(m_hueEdit,0,1);
    hslL->addWidget(lbl("S"),0,2); hslL->addWidget(m_satEdit,0,3);
    hslL->addWidget(lbl("L"),1,0); hslL->addWidget(m_lightEdit,1,1);
    mainLayout->addWidget(hslGroup);

    mainLayout->addStretch();
    scroll->setWidget(content);
    auto* outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->addWidget(scroll);

    connect(m_hexEdit, &QLineEdit::textEdited, this, &ColorPage::onHexChanged);
    connect(m_hexAlphaEdit, &QLineEdit::textEdited, this, &ColorPage::onHexAlphaChanged);
    connect(m_redEdit, &QLineEdit::textEdited, this, &ColorPage::onRgbChanged);
    connect(m_greenEdit, &QLineEdit::textEdited, this, &ColorPage::onRgbChanged);
    connect(m_blueEdit, &QLineEdit::textEdited, this, &ColorPage::onRgbChanged);
    connect(m_alphaEdit, &QLineEdit::textEdited, this, &ColorPage::onRgbChanged);
    connect(m_hueEdit, &QLineEdit::textEdited, this, &ColorPage::onHslChanged);
    connect(m_satEdit, &QLineEdit::textEdited, this, &ColorPage::onHslChanged);
    connect(m_lightEdit, &QLineEdit::textEdited, this, &ColorPage::onHslChanged);
    connect(m_pickBtn, &QPushButton::clicked, this, &ColorPage::onPickColor);
    connect(m_saveHistoryBtn, &QPushButton::clicked, this, &ColorPage::commitToHistory);
    connect(m_colorWheel, &ColorWheel::colorChanged, this, [this](const QColor& c) {
        setColor(c, m_colorWheel);
    });
    connect(m_rgbFormatCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ColorPage::onRgbFormatChanged);
    updateRgbFieldFormat();
    setColor(m_currentColor);
}
void ColorPage::setColor(const QColor& color, QWidget* skip) {
    if (m_updating) return;
    m_updating = true;
    m_currentColor = color;
    if (skip != m_hexEdit) m_hexEdit->setText(color.name(QColor::HexRgb).mid(1).toUpper());
    if (skip != m_hexAlphaEdit) {
        m_hexAlphaEdit->setText(QString("%1%2%3%4")
            .arg(color.red(), 2, 16, QChar('0'))
            .arg(color.green(), 2, 16, QChar('0'))
            .arg(color.blue(), 2, 16, QChar('0'))
            .arg(color.alpha(), 2, 16, QChar('0'))
            .toUpper());
    }
    if (skip != m_redEdit) m_redEdit->setText(formatRgbByte(color.red()));
    if (skip != m_greenEdit) m_greenEdit->setText(formatRgbByte(color.green()));
    if (skip != m_blueEdit) m_blueEdit->setText(formatRgbByte(color.blue()));
    if (skip != m_alphaEdit) m_alphaEdit->setText(formatRgbByte(color.alpha()));
    if (skip != m_hueEdit) m_hueEdit->setText(QString::number(qMax(0, color.hslHue())));
    if (skip != m_satEdit) m_satEdit->setText(QString::number(color.hslSaturation()));
    if (skip != m_lightEdit) m_lightEdit->setText(QString::number(color.lightness()));
    if (skip != m_colorWheel) m_colorWheel->setColor(color);
    updatePreview();
    m_updating = false;
}
void ColorPage::updatePreview() {
    m_preview->setStyleSheet(QString("background-color: %1; border: 1px solid gray; border-radius: 8px;")
        .arg(m_currentColor.name(QColor::HexArgb)));
    m_colorName->setText(Rheno::Core::previewText(m_currentColor, rgbHexMode()));
}
bool ColorPage::rgbHexMode() const {
    return m_rgbFormatCombo->currentIndex() == 1;
}
QString ColorPage::formatRgbByte(int value) const {
    if (rgbHexMode())
        return QString("%1").arg(value, 2, 16, QChar('0')).toUpper();
    return QString::number(value);
}
void ColorPage::updateRgbFieldFormat() {
    const bool hex = rgbHexMode();
    for (QLineEdit* e : {m_redEdit, m_greenEdit, m_blueEdit, m_alphaEdit}) {
        // setValidator() doesn't take ownership of the previous validator, so the
        // old one must be deleted explicitly or it leaks on every format toggle.
        const QValidator* oldValidator = e->validator();
        if (hex) {
            e->setValidator(new QRegularExpressionValidator(QRegularExpression("^[0-9A-Fa-f]{0,2}$"), e));
            e->setMaxLength(2);
        } else {
            e->setValidator(new QIntValidator(0, 255, e));
            e->setMaxLength(3);
        }
        delete oldValidator;
    }
}
void ColorPage::onHexChanged() {
    QColor c;
    if (Rheno::Core::parseHexColor(m_hexEdit->text(), &c))
        setColor(c, m_hexEdit);
}
void ColorPage::onHexAlphaChanged() {
    const QString text = m_hexAlphaEdit->text();
    if (text.length() != 8)
        return;
    bool ok = false;
    const int r = text.mid(0, 2).toInt(&ok, 16); if (!ok) return;
    const int g = text.mid(2, 2).toInt(&ok, 16); if (!ok) return;
    const int b = text.mid(4, 2).toInt(&ok, 16); if (!ok) return;
    const int a = text.mid(6, 2).toInt(&ok, 16); if (!ok) return;
    setColor(QColor(r, g, b, a), m_hexAlphaEdit);
}
void ColorPage::onRgbChanged() {
    setColor(Rheno::Core::fromRgbText(m_redEdit->text(), m_greenEdit->text(), m_blueEdit->text(), m_alphaEdit->text(),
                                       rgbHexMode() ? 16 : 10),
             qobject_cast<QWidget*>(sender()));
}
void ColorPage::onHslChanged() {
    setColor(Rheno::Core::fromHslText(m_hueEdit->text(), m_satEdit->text(), m_lightEdit->text(), m_alphaEdit->text()),
             qobject_cast<QWidget*>(sender()));
}
void ColorPage::onPickColor() {
    QTimer::singleShot(150, this, [this]() {
        auto* picker = new ScreenPicker(this);
        connect(picker, &ScreenPicker::colorPicked, this, [this](const QColor& c) {
            setColor(c);
            showPickStatus(c);
        });
    });
}
void ColorPage::onRgbFormatChanged() {
    updateRgbFieldFormat();
    setColor(m_currentColor);
}
void ColorPage::showPickStatus(const QColor& color) {
    m_pickStatusLabel->setText(QString("\xe2\x9c\x93 Picked: #%1").arg(color.name(QColor::HexRgb).mid(1).toUpper()));
    m_pickStatusLabel->setVisible(true);
    QTimer::singleShot(2500, this, [this]() {
        m_pickStatusLabel->clear();
        m_pickStatusLabel->setVisible(false);
    });
}
void ColorPage::commitToHistory() {
    for (int i = m_history.size() - 1; i >= 0; --i) {
        if (m_history[i].rgba() == m_currentColor.rgba())
            m_history.removeAt(i);
    }
    m_history.prepend(m_currentColor);

    constexpr int kMaxHistory = 10;
    while (m_history.size() > kMaxHistory)
        m_history.removeLast();

    rebuildHistorySwatches();
    saveHistory();
}
void ColorPage::loadHistory() {
    QSettings settings("RhenoCalc", "RhenoCalc");
    const QStringList saved = settings.value("colorHistory").toStringList();
    for (const QString& hex : saved) {
        if (hex.length() != 8)
            continue;
        bool ok = false;
        const int r = hex.mid(0, 2).toInt(&ok, 16); if (!ok) continue;
        const int g = hex.mid(2, 2).toInt(&ok, 16); if (!ok) continue;
        const int b = hex.mid(4, 2).toInt(&ok, 16); if (!ok) continue;
        const int a = hex.mid(6, 2).toInt(&ok, 16); if (!ok) continue;
        m_history.append(QColor(r, g, b, a));
    }
}
void ColorPage::saveHistory() const {
    QStringList out;
    for (const QColor& c : m_history) {
        out << QString("%1%2%3%4")
            .arg(c.red(), 2, 16, QChar('0'))
            .arg(c.green(), 2, 16, QChar('0'))
            .arg(c.blue(), 2, 16, QChar('0'))
            .arg(c.alpha(), 2, 16, QChar('0'))
            .toUpper();
    }
    QSettings settings("RhenoCalc", "RhenoCalc");
    settings.setValue("colorHistory", out);
}
void ColorPage::rebuildHistorySwatches() {
    QLayoutItem* item;
    while ((item = m_historyLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    for (const QColor& c : m_history) {
        auto* btn = new QPushButton;
        btn->setFixedSize(19, 19);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setToolTip(c.name(QColor::HexRgb).toUpper());
        btn->setStyleSheet(QString("background-color: %1; border: 1px solid gray; border-radius: 4px;")
            .arg(c.name(QColor::HexArgb)));
        connect(btn, &QPushButton::clicked, this, [this, c]() { setColor(c); });
        m_historyLayout->addWidget(btn);
    }
    m_historyLayout->addStretch();
    m_historyGroup->setVisible(!m_history.isEmpty());
}
int ColorPage::rgbFormatIndex() const {
    return m_rgbFormatCombo->currentIndex();
}
void ColorPage::setRgbFormatIndex(int index) {
    if (index >= 0 && index < m_rgbFormatCombo->count())
        m_rgbFormatCombo->setCurrentIndex(index);
}
void ColorPage::applyTheme(bool dark) {
    m_isDark = dark;
    updatePreview();
}
#include "colorpage.moc"
