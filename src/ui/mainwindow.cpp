#include "mainwindow.h"
#include "pages/calculatorpage.h"
#include "pages/baseconverterpage.h"
#include "pages/unitconverterpage.h"
#include "themecolors.h"
#include <QApplication>
#include <QStyleFactory>
#include <QFile>
#include <QSettings>
#include <QStatusBar>
#include <QShortcut>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_tabWidget(nullptr)
    , m_calcPage(nullptr)
    , m_basePage(nullptr)
    , m_unitPage(nullptr)
{
    // Theme aus QSettings laden (Standard: dark)
    QSettings settings("RhenoCalc", "RhenoCalc");
    m_isDark = settings.value("darkTheme", true).toBool();

    setupUI();
    applyTheme(m_isDark);
    setWindowTitle("RhenoCalc");
    restoreWindowGeometry();

    // Tab-Navigation mit Shift+Links / Shift+Rechts
    auto* prevTab = new QShortcut(QKeySequence(Qt::SHIFT | Qt::Key_Left), this);
    connect(prevTab, &QShortcut::activated, this, [this]() {
        int idx = m_tabWidget->currentIndex();
        m_tabWidget->setCurrentIndex((idx - 1 + m_tabWidget->count()) % m_tabWidget->count());
        m_calcPage->setFocus();
    });
    auto* nextTab = new QShortcut(QKeySequence(Qt::SHIFT | Qt::Key_Right), this);
    connect(nextTab, &QShortcut::activated, this, [this]() {
        int idx = m_tabWidget->currentIndex();
        m_tabWidget->setCurrentIndex((idx + 1) % m_tabWidget->count());
        m_calcPage->setFocus();
    });

    // Fokus zurück an den Rechner wenn ins Leere geklickt wird
    connect(qApp, &QApplication::focusChanged, this, [this](QWidget* /*old*/, QWidget* now) {
        if (!now && isActiveWindow())
            m_calcPage->setFocus();
    });
}

MainWindow::~MainWindow() {}

void MainWindow::closeEvent(QCloseEvent* event) {
    saveWindowGeometry();
    QMainWindow::closeEvent(event);
}

void MainWindow::showEvent(QShowEvent* event) {
    QMainWindow::showEvent(event);
    m_calcPage->setFocus();
}

void MainWindow::saveWindowGeometry() {
    QSettings settings("RhenoCalc", "RhenoCalc");
    settings.setValue("windowGeometry", saveGeometry());
    settings.setValue("windowState", saveState());
    settings.setValue("darkTheme", m_isDark);
}

void MainWindow::restoreWindowGeometry() {
    QSettings settings("RhenoCalc", "RhenoCalc");
    if (settings.contains("windowGeometry")) {
        restoreGeometry(settings.value("windowGeometry").toByteArray());
        restoreState(settings.value("windowState").toByteArray());
    } else {
        adjustSize();
    }
}

void MainWindow::setupUI() {
    m_tabWidget = new QTabWidget(this);

    m_calcPage = new CalculatorPage(this);
    m_basePage = new BaseConverterPage(this);
    m_unitPage = new UnitConverterPage(this);

    m_tabWidget->addTab(m_calcPage, "Calculator");
    m_tabWidget->addTab(m_basePage, "Base Converter");
    m_tabWidget->addTab(m_unitPage, "Unit Converter");

    // Theme-Toggle-Button oben rechts in der Tab-Leiste
    m_themeBtn = new QPushButton("☀ Light", this);
    m_themeBtn->setFixedHeight(28);
    m_themeBtn->setCursor(Qt::PointingHandCursor);
    m_themeBtn->setFocusPolicy(Qt::NoFocus);
    connect(m_themeBtn, &QPushButton::clicked, this, [this]() {
        m_isDark = !m_isDark;
        applyTheme(m_isDark);
        m_calcPage->setFocus();
    });
    m_tabWidget->setCornerWidget(m_themeBtn, Qt::TopRightCorner);

    setCentralWidget(m_tabWidget);
    statusBar()->showMessage("RhenoCalc  |  Embedded Engineer Toolbox  |  v1.0");
}

void MainWindow::applyTheme(bool dark) {
    m_isDark = dark;

    // ── Globales Stylesheet aus QSS-Datei laden ───────────────────────────────
    QFile qss(dark ? ":/styles/dark.qss" : ":/styles/light.qss");
    if (qss.open(QFile::ReadOnly)) {
        qApp->setStyleSheet(ThemeColors::applyQssColors(QString::fromUtf8(qss.readAll()), dark));
        qss.close();
    }

    // ── Fusion-Palette (Fallback für native Elemente) ─────────────────────────
    qApp->setStyle(QStyleFactory::create("Fusion"));
    qApp->setPalette(ThemeColors::applicationPalette(dark));

    // ── Seiten-spezifische Stile aktualisieren ────────────────────────────────
    m_calcPage->applyTheme(dark);
    m_basePage->applyTheme(dark);
    m_unitPage->applyTheme(dark);

    // ── Status Bar ────────────────────────────────────────────────────────────
    statusBar()->setStyleSheet(ThemeColors::statusBarStyle(dark));

    // ── Theme-Button beschriften ──────────────────────────────────────────────
    if (m_themeBtn) {
        m_themeBtn->setText(dark ? "☀ Light" : "🌙 Dark");
        m_themeBtn->setStyleSheet(ThemeColors::themeToggleButtonStyle(dark));
    }
}
