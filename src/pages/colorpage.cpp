#include "colorpage.h"
#include "ui/themecolors.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QScrollArea>
#include <QSizePolicy>
#include <QClipboard>
#include <QApplication>
#include <QScreen>
#include <QTimer>
#include <QPixmap>
#include <QMouseEvent>
#include <QPainter>
#include <QtMath>

// ─── ColorWheel ───────────────────────────────────────────────────────────────

ColorWheel::ColorWheel(QWidget* parent) : QWidget(parent) {
    setMinimumSize(200, 200);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMouseTracking(false);
}

void ColorWheel::setColor(const QColor& color) {
    m_hue = qMax(0, color.hsvHue());
    m_sat = color.hsvSaturation();
    m_val = color.value();
    update();
}

QColor ColorWheel::color() const {
    return QColor::fromHsv(m_hue, m_sat, m_val);
}

void ColorWheel::rebuildWheel() {
    int side = qMin(width(), height());
    m_wheelImage = QImage(side, side, QImage::Format_ARGB32_Premultiplied);
    m_wheelImage.fill(Qt::transparent);
    int cx = side / 2, cy = side / 2;
    int radius = side / 2 - 2;

    for (int y = 0; y < side; ++y) {
        for (int x = 0; x < side; ++x) {
            int dx = x - cx, dy = y - cy;
            double dist = qSqrt(dx * dx + dy * dy);
            if (dist <= radius) {
                double angle = qAtan2(-dy, dx);
                int hue = static_cast<int>(qRadiansToDegrees(angle));
                if (hue < 0) hue += 360;
                int sat = static_cast<int>(qBound(0.0, dist / radius * 255.0, 255.0));
                m_wheelImage.setPixelColor(x, y, QColor::fromHsv(hue, sat, m_val));
            }
        }
    }
    m_dirty = false;
}

void ColorWheel::paintEvent(QPaintEvent*) {
    if (m_dirty || m_wheelImage.isNull()) rebuildWheel();

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    int side = qMin(width(), height());
    int ox = (width() - side) / 2, oy = (height() - side) / 2;
    p.drawImage(ox, oy, m_wheelImage);

    // Draw selection marker
    int cx = side / 2, cy = side / 2;
    int radius = side / 2 - 2;
    double r = m_sat / 255.0 * radius;
    double angle = qDegreesToRadians(static_cast<double>(m_hue));
    int sx = cx + static_cast<int>(r * qCos(angle));
    int sy = cy - static_cast<int>(r * qSin(angle));

    p.setPen(QPen(Qt::white, 2));
    p.setBrush(Qt::NoBrush);
    p.drawEllipse(QPoint(ox + sx, oy + sy), 6, 6);
    p.setPen(QPen(Qt::black, 1));
    p.drawEllipse(QPoint(ox + sx, oy + sy), 7, 7);
}

void ColorWheel::pickAt(const QPoint& pos) {
    int side = qMin(width(), height());
    int ox = (width() - side) / 2, oy = (height() - side) / 2;
    int cx = side / 2, cy = side / 2;
    int radius = side / 2 - 2;
    int dx = pos.x() - ox - cx, dy = pos.y() - oy - cy;
    double dist = qSqrt(dx * dx + dy * dy);
    if (dist > radius) dist = radius;

    double angle = qAtan2(-dy, dx);
    m_hue = static_cast<int>(qRadiansToDegrees(angle));
    if (m_hue < 0) m_hue += 360;
    m_sat = static_cast<int>(qBound(0.0, dist / radius * 255.0, 255.0));

    update();
    emit colorChanged(color());
}

void ColorWheel::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) pickAt(event->pos());
}

void ColorWheel::mouseMoveEvent(QMouseEvent* event) {
    if (event->buttons() & Qt::LeftButton) pickAt(event->pos());
}

void ColorWheel::resizeEvent(QResizeEvent*) {
    m_dirty = true;
}
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
        if (event->button() == Qt::LeftButton)
            emit colorPicked(m_screenshot.toImage().pixelColor(event->pos()));
        close(); deleteLater();
    }
    void keyPressEvent(QKeyEvent* event) override {
        if (event->key() == Qt::Key_Escape) { close(); deleteLater(); }
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
    m_colorWheel->setFixedSize(220, 220);
    mainLayout->addWidget(m_colorWheel, 0, Qt::AlignHCenter);

    m_preview = new QLabel;
    m_preview->setFixedHeight(80);
    m_preview->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_preview);
    m_colorName = new QLabel;
    m_colorName->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_colorName);
    auto* hexGroup = new QGroupBox("HEX");
    auto* hexLayout = new QHBoxLayout(hexGroup);
    m_hexEdit = new QLineEdit("3498DB");
    m_hexEdit->setMaxLength(8);
    hexLayout->addWidget(new QLabel("#"));
    hexLayout->addWidget(m_hexEdit);
    m_pickBtn = new QPushButton("\xe2\x8a\x99 Pick");
    m_pickBtn->setFixedWidth(70);
    m_pickBtn->setToolTip("Pick a color from screen");
    hexLayout->addWidget(m_pickBtn);
    mainLayout->addWidget(hexGroup);
    auto mk = [](int max) { auto* s = new QSpinBox; s->setRange(0, max); s->setAlignment(Qt::AlignCenter); return s; };
    auto lbl = [](const QString& t) { auto* l = new QLabel(t); l->setFixedWidth(l->fontMetrics().horizontalAdvance(t) + 4); return l; };
    auto* rgbGroup = new QGroupBox("RGB");
    auto* rgbL = new QGridLayout(rgbGroup);
    m_redSpin = mk(255); m_greenSpin = mk(255); m_blueSpin = mk(255); m_alphaSpin = mk(255);
    m_alphaSpin->setValue(255);
    rgbL->addWidget(lbl("R"),0,0); rgbL->addWidget(m_redSpin,0,1);
    rgbL->addWidget(lbl("G"),0,2); rgbL->addWidget(m_greenSpin,0,3);
    rgbL->addWidget(lbl("B"),1,0); rgbL->addWidget(m_blueSpin,1,1);
    rgbL->addWidget(lbl("A"),1,2); rgbL->addWidget(m_alphaSpin,1,3);
    mainLayout->addWidget(rgbGroup);
    auto* hslGroup = new QGroupBox("HSL");
    auto* hslL = new QGridLayout(hslGroup);
    m_hueSpin = mk(359); m_satSpin = mk(255); m_lightSpin = mk(255);
    hslL->addWidget(lbl("H"),0,0); hslL->addWidget(m_hueSpin,0,1);
    hslL->addWidget(lbl("S"),0,2); hslL->addWidget(m_satSpin,0,3);
    hslL->addWidget(lbl("L"),1,0); hslL->addWidget(m_lightSpin,1,1);
    mainLayout->addWidget(hslGroup);
    mainLayout->addStretch();
    scroll->setWidget(content);
    auto* outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->addWidget(scroll);
    connect(m_hexEdit, &QLineEdit::textEdited, this, &ColorPage::onHexChanged);
    connect(m_redSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &ColorPage::onRgbChanged);
    connect(m_greenSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &ColorPage::onRgbChanged);
    connect(m_blueSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &ColorPage::onRgbChanged);
    connect(m_alphaSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &ColorPage::onRgbChanged);
    connect(m_hueSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &ColorPage::onHslChanged);
    connect(m_satSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &ColorPage::onHslChanged);
    connect(m_lightSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &ColorPage::onHslChanged);
    connect(m_pickBtn, &QPushButton::clicked, this, &ColorPage::onPickColor);
    connect(m_colorWheel, &ColorWheel::colorChanged, this, [this](const QColor& c) {
        setColor(c, m_colorWheel);
    });
    setColor(m_currentColor);
}
void ColorPage::setColor(const QColor& color, QWidget* skip) {
    if (m_updating) return;
    m_updating = true;
    m_currentColor = color;
    if (skip != m_hexEdit) m_hexEdit->setText(color.name(QColor::HexRgb).mid(1).toUpper());
    if (skip != m_redSpin) m_redSpin->setValue(color.red());
    if (skip != m_greenSpin) m_greenSpin->setValue(color.green());
    if (skip != m_blueSpin) m_blueSpin->setValue(color.blue());
    if (skip != m_alphaSpin) m_alphaSpin->setValue(color.alpha());
    if (skip != m_hueSpin) m_hueSpin->setValue(qMax(0, color.hslHue()));
    if (skip != m_satSpin) m_satSpin->setValue(color.hslSaturation());
    if (skip != m_lightSpin) m_lightSpin->setValue(color.lightness());
    if (skip != m_colorWheel) m_colorWheel->setColor(color);
    updatePreview();
    m_updating = false;
}
void ColorPage::updatePreview() {
    m_preview->setStyleSheet(QString("background-color: %1; border: 1px solid gray; border-radius: 8px;")
        .arg(m_currentColor.name(QColor::HexArgb)));
    m_colorName->setText(QString("HEX: #%1  |  RGB(%2, %3, %4)  |  HSL(%5, %6%, %7%)")
        .arg(m_currentColor.name(QColor::HexRgb).mid(1).toUpper())
        .arg(m_currentColor.red()).arg(m_currentColor.green()).arg(m_currentColor.blue())
        .arg(qMax(0, m_currentColor.hslHue()))
        .arg(qRound(m_currentColor.hslSaturationF() * 100))
        .arg(qRound(m_currentColor.lightnessF() * 100)));
}
void ColorPage::onHexChanged() {
    QString hex = m_hexEdit->text().trimmed();
    if (!hex.startsWith('#')) hex.prepend('#');
    QColor c(hex);
    if (c.isValid()) setColor(c, m_hexEdit);
}
void ColorPage::onRgbChanged() {
    setColor(QColor(m_redSpin->value(), m_greenSpin->value(), m_blueSpin->value(), m_alphaSpin->value()),
             qobject_cast<QWidget*>(sender()));
}
void ColorPage::onHslChanged() {
    setColor(QColor::fromHsl(m_hueSpin->value(), m_satSpin->value(), m_lightSpin->value(), m_alphaSpin->value()),
             qobject_cast<QWidget*>(sender()));
}
void ColorPage::onPickColor() {
    QTimer::singleShot(150, this, [this]() {
        auto* picker = new ScreenPicker(this);
        connect(picker, &ScreenPicker::colorPicked, this, [this](const QColor& c) { setColor(c); });
    });
}
void ColorPage::applyTheme(bool dark) {
    m_isDark = dark;
    updatePreview();
}
#include "colorpage.moc"
