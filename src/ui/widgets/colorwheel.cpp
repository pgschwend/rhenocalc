#include "colorwheel.h"

#include <QMouseEvent>
#include <QPainter>
#include <QSizePolicy>
#include <QtMath>

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

