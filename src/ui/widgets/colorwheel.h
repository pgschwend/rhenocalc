#pragma once

#include <QWidget>
#include <QColor>
#include <QImage>

// HSV color wheel: X = Hue (angle), Y = Saturation (radius), separate Value slider
class ColorWheel : public QWidget {
    Q_OBJECT
public:
    explicit ColorWheel(QWidget* parent = nullptr);
    void setColor(const QColor& color);
    QColor color() const;
signals:
    void colorChanged(const QColor& color);
protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void resizeEvent(QResizeEvent*) override;
private:
    void rebuildWheel();
    void pickAt(const QPoint& pos);
    QImage m_wheelImage;
    int m_hue = 210;
    int m_sat = 200;
    int m_val = 219;
    bool m_dirty = true;
};

