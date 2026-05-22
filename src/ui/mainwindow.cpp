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
        "QTabWidget::pane { border: none; background: #3c3c3c; }"
        "QTabBar::tab { background: #4a4a4a; color: #cccccc; padding: 10px 22px; font-size: 13px; border-radius: 0; }"
        "QTabBar::tab:selected { background: #3c3c3c; color: #ffffff; border-bottom: 2px solid #aaaaaa; }"
        "QTabBar::tab:hover { background: #575757; color: #ffffff; }"
    );

    m_calcPage = new CalculatorPage(this);
    m_basePage = new BaseConverterPage(this);
    m_unitPage = new UnitConverterPage(this);

    m_tabWidget->addTab(m_calcPage, "Calculator");
    m_tabWidget->addTab(m_basePage, "Base Converter");
    m_tabWidget->addTab(m_unitPage, "Unit Converter");

    setCentralWidget(m_tabWidget);

    statusBar()->showMessage("RhenoCalc  |  Embedded Engineer Toolbox  |  v1.0");
    statusBar()->setStyleSheet("background:#4a4a4a;color:#bbbbbb;font-size:11px;");
}

void MainWindow::applyTheme() {
    qApp->setStyle(QStyleFactory::create("Fusion"));
    QPalette p;
    p.setColor(QPalette::Window,          QColor("#3c3c3c"));
    p.setColor(QPalette::WindowText,      QColor("#f0f0f0"));
    p.setColor(QPalette::Base,            QColor("#444444"));
    p.setColor(QPalette::AlternateBase,   QColor("#4e4e4e"));
    p.setColor(QPalette::Text,            QColor("#f0f0f0"));
    p.setColor(QPalette::Button,          QColor("#4a4a4a"));
    p.setColor(QPalette::ButtonText,      QColor("#f0f0f0"));
    p.setColor(QPalette::Highlight,       QColor("#888888"));
    p.setColor(QPalette::HighlightedText, QColor("#ffffff"));
    qApp->setPalette(p);
}
