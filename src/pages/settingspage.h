#pragma once

#include <QWidget>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>
#include <QGroupBox>
#include <QVBoxLayout>

class MainWindow;

class SettingsPage : public QWidget {
    Q_OBJECT
public:
    explicit SettingsPage(MainWindow* mainWindow, QWidget* parent = nullptr);
    void applyTheme(bool dark);
    void updateStatusBar(const QString& updateVersion = QString(), const QString& releaseUrl = QString());

signals:
    void themeChanged(bool dark);
    void closeWithEscChanged(bool enabled);
    void restoreTabIndexChanged(bool enabled);
    void windowPositionChanged(int mode);

private:
    void setupUI();

    MainWindow* m_mainWindow;
    QPushButton* m_themeBtn;
    QCheckBox*   m_closeWithEscCheck;
    QCheckBox*   m_restoreTabIndexCheck;
    QComboBox*   m_windowPosCombo;
    QLabel*      m_versionLabel;
    QLabel*      m_versionUpdateAvailable;

    QGroupBox* m_appearanceGroup;
    QGroupBox* m_windowGroup;
    QGroupBox* m_aboutGroup;
    QVBoxLayout* m_aboutLayout;

    QString m_updateText;
    QString m_updateVersion;
    QString m_updateUrl;

    bool m_isDark = true;
};

