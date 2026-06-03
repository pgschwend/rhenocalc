#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QCloseEvent>
#include <QShowEvent>
#include <QPushButton>
#include <QMenu>
#include <QVector>
#include <QPair>
#include <QTabBar>

class CalculatorPage;
class BaseConverterPage;
class UnitConverterPage;
class NetworkPage;
class CrcHashPage;
class ColorPage;

// Custom tab bar with fixed tab widths
class FixedTabBar : public QTabBar {
public:
    using QTabBar::QTabBar;
    QSize tabSizeHint(int index) const override {
        QSize s = QTabBar::tabSizeHint(index);
        if (index <= 1)       s.setWidth(58);   // Calc, Base
        else if (index == 2)  s.setWidth(86);   // dynamic tab (wider)
        else if (index == 3)  s.setWidth(36);   // "▾" (small)
        return s;
    }
};

// QTabWidget that uses the FixedTabBar
class FixedTabWidget : public QTabWidget {
public:
    explicit FixedTabWidget(QWidget* parent = nullptr) : QTabWidget(parent) {
        setTabBar(new FixedTabBar(this));
    }
};

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
    void switchDynamicTab(QWidget* page, const QString& title);
    void saveWindowGeometry();
    void restoreWindowGeometry();

    QTabWidget*        m_tabWidget;
    CalculatorPage*    m_calcPage;
    BaseConverterPage* m_basePage;
    UnitConverterPage* m_unitPage;
    NetworkPage*       m_networkPage;
    CrcHashPage*       m_crcHashPage;
    ColorPage*         m_colorPage;
    QPushButton*       m_onTopBtn = nullptr;
    QPushButton*       m_themeBtn = nullptr;
    QMenu*             m_moreMenu = nullptr;

    // Extra pages available via "More" tab
    QVector<QPair<QString, QWidget*>> m_extraPages;

    bool               m_isDark   = true;
    bool               m_alwaysOnTop = false;
};

#endif // MAINWINDOW_H
