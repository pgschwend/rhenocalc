#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QCloseEvent>
#include <QShowEvent>
#include <QPushButton>

class CalculatorPage;
class BaseConverterPage;
class UnitConverterPage;
class NetworkPage;
class CrcHashPage;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent* event) override;
    void showEvent(QShowEvent* event) override;

private:
    void setupUI();
    void applyTheme(bool dark);
    void applyAlwaysOnTop(bool enabled, bool persist);
    void updateOnTopButton();
    void saveWindowGeometry();
    void restoreWindowGeometry();

    QTabWidget*        m_tabWidget;
    CalculatorPage*    m_calcPage;
    BaseConverterPage* m_basePage;
    UnitConverterPage* m_unitPage;
    NetworkPage*       m_networkPage;
    CrcHashPage*       m_crcHashPage;
    QPushButton*       m_onTopBtn = nullptr;
    QPushButton*       m_themeBtn = nullptr;
    bool               m_isDark   = true;
    bool               m_alwaysOnTop = false;
};

#endif // MAINWINDOW_H
