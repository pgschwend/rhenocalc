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
#include <QIntValidator>

// ─── ColorWheel ───────────────────────────────────────────────────────────────

ColorWheel::ColorWheel(QWidget* parent) : QWidget(parent) {
    setMinimumSize(100, 100);
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
    auto* hexGroup = new QGroupBox("HEX");
    auto* hexLayout = new QHBoxLayout(hexGroup);
    m_hexEdit = new QLineEdit("3498DB");
    m_hexEdit->setMaxLength(8);
    hexLayout->addWidget(new QLabel("#"));
    hexLayout->addWidget(m_hexEdit);
    m_pickBtn = new QPushButton("\xe2\x8a\x99 Pick");
    m_pickBtn->setMinimumWidth(60);
    m_pickBtn->setToolTip("Pick a color from screen");
    hexLayout->addWidget(m_pickBtn);
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
    connect(m_redEdit, &QLineEdit::textEdited, this, &ColorPage::onRgbChanged);
    connect(m_greenEdit, &QLineEdit::textEdited, this, &ColorPage::onRgbChanged);
    connect(m_blueEdit, &QLineEdit::textEdited, this, &ColorPage::onRgbChanged);
    connect(m_alphaEdit, &QLineEdit::textEdited, this, &ColorPage::onRgbChanged);
    connect(m_hueEdit, &QLineEdit::textEdited, this, &ColorPage::onHslChanged);
    connect(m_satEdit, &QLineEdit::textEdited, this, &ColorPage::onHslChanged);
    connect(m_lightEdit, &QLineEdit::textEdited, this, &ColorPage::onHslChanged);
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
    if (skip != m_redEdit) m_redEdit->setText(QString::number(color.red()));
    if (skip != m_greenEdit) m_greenEdit->setText(QString::number(color.green()));
    if (skip != m_blueEdit) m_blueEdit->setText(QString::number(color.blue()));
    if (skip != m_alphaEdit) m_alphaEdit->setText(QString::number(color.alpha()));
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
    m_colorName->setText(QString("RGB(%1, %2, %3)  |  HSL(%4, %5%, %6%)")
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
    setColor(QColor(m_redEdit->text().toInt(), m_greenEdit->text().toInt(),
                    m_blueEdit->text().toInt(), m_alphaEdit->text().toInt()),
             qobject_cast<QWidget*>(sender()));
}
void ColorPage::onHslChanged() {
    setColor(QColor::fromHsl(m_hueEdit->text().toInt(), m_satEdit->text().toInt(),
                             m_lightEdit->text().toInt(), m_alphaEdit->text().toInt()),
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
