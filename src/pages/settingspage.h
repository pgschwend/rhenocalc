#pragma once

#include <QWidget>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>
#include <QGroupBox>

class MainWindow;

class SettingsPage : public QWidget {
    Q_OBJECT
public:
    explicit SettingsPage(MainWindow* mainWindow, QWidget* parent = nullptr);
    void applyTheme(bool dark);

signals:
    void themeChanged(bool dark);
    void alwaysOnTopChanged(bool enabled);
    void windowPositionChanged(int mode);

private:
    void setupUI();

    MainWindow* m_mainWindow;
    QPushButton* m_themeBtn;
    QCheckBox*   m_alwaysOnTopCheck;
    QComboBox*   m_windowPosCombo;
    QLabel*      m_versionLabel;

    QGroupBox* m_appearanceGroup;
    QGroupBox* m_windowGroup;
    QGroupBox* m_aboutGroup;

    bool m_isDark = true;
};

