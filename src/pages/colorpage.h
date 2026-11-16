#pragma once

#include "ui/widgets/colorwheel.h"
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QColor>
#include <QList>

class QHBoxLayout;
class QGroupBox;

class ColorPage : public QWidget {
    Q_OBJECT
public:
    explicit ColorPage(QWidget* parent = nullptr);
    void applyTheme(bool dark);

    int rgbFormatIndex() const;
    void setRgbFormatIndex(int index);

private slots:
    void onHexChanged();
    void onHexAlphaChanged();
    void onRgbChanged();
    void onHslChanged();
    void onPickColor();
    void onRgbFormatChanged();
    void commitToHistory();

private:
    void setupUI();
    void setColor(const QColor& color, QWidget* skip = nullptr);
    void updatePreview();
    void updateRgbFieldFormat();
    void rebuildHistorySwatches();
    void loadHistory();
    void saveHistory() const;
    void showPickStatus(const QColor& color);
    bool rgbHexMode() const;
    QString formatRgbByte(int value) const;

    QLineEdit* m_hexEdit;
    QLineEdit* m_hexAlphaEdit;
    QLineEdit* m_redEdit;
    QLineEdit* m_greenEdit;
    QLineEdit* m_blueEdit;
    QLineEdit* m_alphaEdit;
    QLineEdit* m_hueEdit;
    QLineEdit* m_satEdit;
    QLineEdit* m_lightEdit;
    QLabel*    m_preview;
    QLabel*    m_colorName;
    QLabel*    m_pickStatusLabel;
    QPushButton* m_pickBtn;
    QPushButton* m_saveHistoryBtn;
    QComboBox*   m_rgbFormatCombo;
    ColorWheel*  m_colorWheel;

    QGroupBox*   m_historyGroup;
    QHBoxLayout* m_historyLayout;
    QList<QColor> m_history;

    QColor m_currentColor;
    bool   m_updating = false;
    bool   m_isDark = true;
};
