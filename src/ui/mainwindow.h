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
    void applyAlwaysOnTop(bool enabled, bool persist);
    void updateOnTopButton();
    void updateStatusBar(const QString& updateVersion = QString(), const QString& releaseUrl = QString());
    void switchDynamicTab(QWidget* page, const QString& title);
    void saveWindowGeometry();
    void restoreWindowGeometry();

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
    QPushButton*       m_onTopBtn = nullptr;
    QPushButton*       m_themeBtn = nullptr;
    QMenu*             m_moreMenu = nullptr;
    QLabel*            m_statusLabel = nullptr;

    QVector<QPair<QString, QWidget*>> m_extraPages;

    bool m_isDark = true;
    bool m_alwaysOnTop = false;
    QString m_updateVersion;
    QString m_updateUrl;
};

