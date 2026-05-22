#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>

class CalculatorPage;
class BaseConverterPage;
class UnitConverterPage;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private:
    void setupUI();
    void applyTheme();

    QTabWidget*       m_tabWidget;
    CalculatorPage*   m_calcPage;
    BaseConverterPage* m_basePage;
    UnitConverterPage* m_unitPage;
};

#endif // MAINWINDOW_H

