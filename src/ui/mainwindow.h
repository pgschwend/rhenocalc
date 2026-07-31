#pragma once

#include <QMainWindow>
#include <QShowEvent>
#include <QPushButton>

class CalculatorPage;
class BaseConverterPage;
class UnitConverterPage;
class NetworkPage;
class CrcHashPage;
class ColorPage;
class FinancePage;
class FloatPage;
class ElectronicsPage;
class SettingsPage;
class TabCoordinator;
class QLabel;
class QTabWidget;


class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    void setupUI();
    void applyTheme(bool dark);
    void applyTitleBarTheme(bool dark);
    void applyAlwaysOnTop(bool enabled, bool persist);
    void updateOnTopButton();
    void saveToolSettings();
    void restoreToolSettings();
    void restoreUISettings();
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
    SettingsPage*      m_settingsPage = nullptr;
    TabCoordinator*    m_tabCoordinator = nullptr;
    QPushButton*       m_onTopBtn = nullptr;
    QLabel*            m_statusLabel = nullptr;

    enum class WindowStartPosition {
        LastPosition,
        CenterOnScreen,
        AtMousePosition
    };

    WindowStartPosition m_windowStartPosition = WindowStartPosition::CenterOnScreen;

    bool m_isDark = true;
    bool m_alwaysOnTop = false;
    bool m_closeWithEsc = false;
    QString m_updateVersion;
    QString m_updateUrl;
};

