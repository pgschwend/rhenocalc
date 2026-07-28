#pragma once

#include <QMainWindow>
#include <QShowEvent>
#include <QPushButton>
#include <QMenu>
#include <QVector>

class CalculatorPage;
class BaseConverterPage;
class UnitConverterPage;
class NetworkPage;
class CrcHashPage;
class ColorPage;
class FinancePage;
class FloatPage;
class ElectronicsPage;
class NetToolsPage;
class SettingsPage;
class QLabel;


class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent* event) override;
    void showEvent(QShowEvent* event) override;

private:
    void setupUI();
    void applyTheme(bool dark);
    void applyTitleBarTheme(bool dark);
    void applyAlwaysOnTop(bool enabled, bool persist);
    void updateOnTopButton();
    void updateStatusBar(const QString& updateVersion = QString(), const QString& releaseUrl = QString());
    void switchDynamicTab(QWidget* page, const QString& title);
    void saveWindowGeometry();
    void restoreWindowGeometry();
    void moveToMousePosition();
    void centerOnMouseScreen();
    void setWindowPosition();

    QTabWidget*        m_tabWidget = nullptr;
    CalculatorPage*    m_calcPage = nullptr;
    BaseConverterPage* m_basePage = nullptr;
    UnitConverterPage* m_unitPage = nullptr;
    NetworkPage*       m_networkPage = nullptr;
    CrcHashPage*       m_crcHashPage = nullptr;
    ColorPage*         m_colorPage = nullptr;
    FinancePage*       m_financePage = nullptr;
    FloatPage*         m_floatPage = nullptr;
    ElectronicsPage*   m_electronicsPage = nullptr;
    NetToolsPage*      m_netToolsPage = nullptr;
    SettingsPage*      m_settingsPage = nullptr;
    QPushButton*       m_onTopBtn = nullptr;
    QMenu*             m_moreMenu = nullptr;
    QLabel*            m_statusLabel = nullptr;

    QVector<QPair<QString, QWidget*>> m_extraPages;

    enum class WindowStartPosition {
        LastPosition,
        CenterOnScreen,
        AtMousePosition
    };

    WindowStartPosition m_windowStartPosition = WindowStartPosition::CenterOnScreen;

    bool m_isDark = true;
    bool m_alwaysOnTop = false;
    bool m_menuJustClosed = false;
    QString m_updateVersion;
    QString m_updateUrl;
};

