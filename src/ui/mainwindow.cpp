#include "mainwindow.h"
#include "pages/calculatorpage.h"
#include "pages/baseconverterpage.h"
#include "pages/unitconverterpage.h"
#include "pages/networkpage.h"
#include "themecolors.h"
#include <QApplication>
#include <QStyleFactory>
#include <QFile>
#include <QSettings>
#include <QStatusBar>
#include <QShortcut>
#include <QHBoxLayout>
#include <QWidget>
#include <QGuiApplication>
#include <QIcon>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_tabWidget(nullptr)
    , m_calcPage(nullptr)
    , m_basePage(nullptr)
    , m_unitPage(nullptr)
    , m_networkPage(nullptr)
{
    // Load theme from QSettings (default: dark)
    QSettings settings("RhenoCalc", "RhenoCalc");
    m_isDark = settings.value("darkTheme", true).toBool();
    m_alwaysOnTop = settings.value("alwaysOnTop", false).toBool();

    setupUI();
    applyTheme(m_isDark);
    setWindowTitle("RhenoCalc");
    restoreWindowGeometry();
    // Apply after restoreState/restoreGeometry so restored state does not override the hint.
    applyAlwaysOnTop(m_alwaysOnTop, false);

    // Tab navigation with Shift+Left / Shift+Right
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

    // Return focus to the calculator when clicking on empty area
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
    settings.setValue("alwaysOnTop", m_alwaysOnTop);
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
    m_networkPage = new NetworkPage(this);

    m_tabWidget->addTab(m_calcPage, "Calc");
    m_tabWidget->addTab(m_basePage, "Base");
    m_tabWidget->addTab(m_unitPage, "Unit");
    m_tabWidget->addTab(m_networkPage, "Network");

    // Buttons top right in the tab bar
    m_onTopBtn = new QPushButton(this);
    m_onTopBtn->setCheckable(true);
    m_onTopBtn->setFixedSize(32, 26);
    m_onTopBtn->setCursor(Qt::PointingHandCursor);
    m_onTopBtn->setFocusPolicy(Qt::NoFocus);
    m_onTopBtn->setIcon(QIcon(":/icons/pin_gray.svg"));
    m_onTopBtn->setIconSize(QSize(14, 14));
    connect(m_onTopBtn, &QPushButton::clicked, this, [this]() {
        applyAlwaysOnTop(!m_alwaysOnTop, true);
        m_calcPage->setFocus();
    });

    m_themeBtn = new QPushButton("☀", this);
    m_themeBtn->setFixedSize(32, 26);
    m_themeBtn->setCursor(Qt::PointingHandCursor);
    m_themeBtn->setFocusPolicy(Qt::NoFocus);
    connect(m_themeBtn, &QPushButton::clicked, this, [this]() {
        m_isDark = !m_isDark;
        applyTheme(m_isDark);
        m_calcPage->setFocus();
    });
    auto* corner = new QWidget(this);
    auto* cornerLayout = new QHBoxLayout(corner);
    cornerLayout->setContentsMargins(0, 0, 4, 0);
    cornerLayout->setSpacing(4);
    cornerLayout->addWidget(m_onTopBtn);
    cornerLayout->addWidget(m_themeBtn);
    m_tabWidget->setCornerWidget(corner, Qt::TopRightCorner);

    setCentralWidget(m_tabWidget);
    statusBar()->showMessage("RhenoCalc  |  Embedded Engineering Toolbox  |  v1.0");
}

void MainWindow::applyTheme(bool dark) {
    m_isDark = dark;

    // ── Load global stylesheet from QSS file ─────────────────────────────────
    QFile qss(dark ? ":/styles/dark.qss" : ":/styles/light.qss");
    if (qss.open(QFile::ReadOnly)) {
        qApp->setStyleSheet(ThemeColors::applyQssColors(QString::fromUtf8(qss.readAll()), dark));
        qss.close();
    }

    // ── Fusion palette (fallback for native elements) ─────────────────────────
    qApp->setStyle(QStyleFactory::create("Fusion"));
    qApp->setPalette(ThemeColors::applicationPalette(dark));

    // ── Update page-specific styles ────────────────────────────────────────────
    m_calcPage->applyTheme(dark);
    m_basePage->applyTheme(dark);
    m_unitPage->applyTheme(dark);
    m_networkPage->applyTheme(dark);

    // ── Status Bar ────────────────────────────────────────────────────────────
    statusBar()->setStyleSheet(ThemeColors::statusBarStyle(dark));

    // ── Update theme button label ─────────────────────────────────────────────
    if (m_themeBtn) {
        m_themeBtn->setText(dark ? "☀" : "🌙");
        m_themeBtn->setStyleSheet(ThemeColors::themeToggleButtonStyle(dark));
    }

    updateOnTopButton();
}

void MainWindow::applyAlwaysOnTop(bool enabled, bool persist) {
    m_alwaysOnTop = enabled;

    if (persist) {
        QSettings settings("RhenoCalc", "RhenoCalc");
        settings.setValue("alwaysOnTop", m_alwaysOnTop);
    }

    const bool wasVisible = isVisible();
    const bool wasMaximized = isMaximized();
    const bool wasFullScreen = isFullScreen();
    const QRect normalGeo = normalGeometry().isValid() ? normalGeometry() : geometry();

    const QString platform = QGuiApplication::platformName().toLower();
    const bool isX11 = platform.contains("xcb");
    const bool isWayland = platform.contains("wayland");

    // On Linux/WMs, setting individual flags and re-showing is more reliable than setWindowFlags().
    setWindowFlag(Qt::WindowStaysOnTopHint, enabled);
    setWindowFlag(Qt::X11BypassWindowManagerHint, enabled && isX11);

    if (wasVisible) {
        if (wasFullScreen) {
            showFullScreen();
        } else if (wasMaximized) {
            showMaximized();
        } else {
            showNormal();
            setGeometry(normalGeo);
        }
    }

    if (isVisible()) {
        raise();
        activateWindow();
    }

    if (enabled && isWayland)
        statusBar()->showMessage("Always-on-top on Wayland may be ignored by the compositor.", 5000);

    updateOnTopButton();
}

void MainWindow::updateOnTopButton() {
    if (!m_onTopBtn)
        return;

    m_onTopBtn->setChecked(m_alwaysOnTop);
    m_onTopBtn->setToolTip(m_alwaysOnTop ? "Always on top: ON" : "Always on top: OFF");

    QString style = ThemeColors::themeToggleButtonStyle(m_isDark);
    if (m_alwaysOnTop) {
        if (m_isDark) {
            style += "QPushButton{border:2px solid #dfdfdf;}";
        }
        else {
            style += "QPushButton{border:2px solid #f1c40f;}";
        }
    }
    m_onTopBtn->setStyleSheet(style);
}
