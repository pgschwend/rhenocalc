#include "mainwindow.h"
#include "pages/calculatorpage.h"
#include "pages/baseconverterpage.h"
#include "pages/unitconverterpage.h"
#include <QApplication>
#include <QStyleFactory>
#include <QFile>
#include <QVBoxLayout>
#include <QSettings>
#include <QDebug>
#include <QLabel>
#include <QStatusBar>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_tabWidget(nullptr)
    , m_calcPage(nullptr)
    , m_basePage(nullptr)
    , m_unitPage(nullptr)
{
    setupUI();
    applyTheme();
    setWindowTitle("RhenoCalc – Embedded Engineer Calculator");
    resize(900, 700);
}

MainWindow::~MainWindow() {}

void MainWindow::setupUI() {
    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setStyleSheet(
        "QTabWidget::pane { border: none; background: #1a1d27; }"
        "QTabBar::tab { background: #2d3240; color: #aaa; padding: 10px 22px; font-size: 13px; border-radius: 0; }"
        "QTabBar::tab:selected { background: #1a1d27; color: #00e5ff; border-bottom: 2px solid #00e5ff; }"
        "QTabBar::tab:hover { background: #3a3f52; color: white; }"
    );

    m_calcPage = new CalculatorPage(this);
    m_basePage = new BaseConverterPage(this);
    m_unitPage = new UnitConverterPage(this);

    m_tabWidget->addTab(m_calcPage, "Calculator");
    m_tabWidget->addTab(m_basePage, "Base Converter");
    m_tabWidget->addTab(m_unitPage, "Unit Converter");

    setCentralWidget(m_tabWidget);

    statusBar()->showMessage("RhenoCalc  |  Embedded Engineer Toolbox  |  v1.0");
    statusBar()->setStyleSheet("background:#12141c;color:#666;font-size:11px;");
}

void MainWindow::applyTheme() {
    qApp->setStyle(QStyleFactory::create("Fusion"));
    QPalette p;
    p.setColor(QPalette::Window,          QColor("#1a1d27"));
    p.setColor(QPalette::WindowText,      QColor("#e0e0e0"));
    p.setColor(QPalette::Base,            QColor("#1e222c"));
    p.setColor(QPalette::AlternateBase,   QColor("#2a2d3a"));
    p.setColor(QPalette::Text,            QColor("#e0e0e0"));
    p.setColor(QPalette::Button,          QColor("#2d3240"));
    p.setColor(QPalette::ButtonText,      QColor("#e0e0e0"));
    p.setColor(QPalette::Highlight,       QColor("#00b8d4"));
    p.setColor(QPalette::HighlightedText, QColor("#000000"));
    qApp->setPalette(p);
}
