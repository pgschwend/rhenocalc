#pragma once
#include <QWidget>
#include <QLineEdit>
#include <QLabel>
#include <QSpinBox>
#include <QPushButton>
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

class ColorPage : public QWidget {
    Q_OBJECT
public:
    explicit ColorPage(QWidget* parent = nullptr);
    void applyTheme(bool dark);

private slots:
    void onHexChanged();
    void onRgbChanged();
    void onHslChanged();
    void onPickColor();

private:
    void setupUI();
    void setColor(const QColor& color, QWidget* skip = nullptr);
    void updatePreview();

    QLineEdit* m_hexEdit;
    QSpinBox*  m_redSpin;
    QSpinBox*  m_greenSpin;
    QSpinBox*  m_blueSpin;
    QSpinBox*  m_alphaSpin;
    QSpinBox*  m_hueSpin;
    QSpinBox*  m_satSpin;
    QSpinBox*  m_lightSpin;
    QLabel*    m_preview;
    QLabel*    m_colorName;
    QPushButton* m_pickBtn;
    ColorWheel*  m_colorWheel;

    QColor m_currentColor;
    bool   m_updating = false;
    bool   m_isDark = true;
};
