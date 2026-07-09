#pragma once

#include "ui/widgets/colorwheel.h"
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QColor>

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
    QLineEdit* m_redEdit;
    QLineEdit* m_greenEdit;
    QLineEdit* m_blueEdit;
    QLineEdit* m_alphaEdit;
    QLineEdit* m_hueEdit;
    QLineEdit* m_satEdit;
    QLineEdit* m_lightEdit;
    QLabel*    m_preview;
    QLabel*    m_colorName;
    QPushButton* m_pickBtn;
    ColorWheel*  m_colorWheel;

    QColor m_currentColor;
    bool   m_updating = false;
    bool   m_isDark = true;
};
