#include "mainwindow.h"
#include "calculatorpage.h"
#include "baseconverterpage.h"
#include "unitconverterpage.h"
#include "networkpage.h"
#include "crchashpage.h"
#include "colorpage.h"
#include "financepage.h"
#include "floatpage.h"
#include "electronicspage.h"
#include "settingspage.h"
#include "themecolors.h"
#include "info.h"
#include "core/updater.h"
#include "fixedtab.h"
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
#include <QLabel>
#include <QTimer>

#include <QGuiApplication>
#include <QScreen>
#include <QCursor>


MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    // Load theme from QSettings (default: dark)
    QSettings settings("RhenoCalc", "RhenoCalc");
    m_isDark = settings.value("darkTheme", true).toBool();
    m_alwaysOnTop = settings.value("alwaysOnTop", false).toBool();
    int positionMode = settings.value("windowStartPosition", 1).toInt();
    m_windowStartPosition = static_cast<WindowStartPosition>(positionMode);

    setupUI();
    applyTheme(m_isDark);
    setWindowTitle("RhenoCalc");
    restoreWindowGeometry();
    setWindowPosition();

    // Apply after restoreState/restoreGeometry so restored state does not override the hint.
    applyAlwaysOnTop(m_alwaysOnTop, false);

    // Tab navigation with Ctrl+Left / Ctrl+Right
    // Virtual order: Calc(0), Base(1), then all extra pages on the dynamic tab (index 2)
    auto currentVirtual = std::make_shared<int>(0);

    auto navigateTab = [this, currentVirtual](int direction) {
        const int totalPages = 2 + static_cast<int>(m_extraPages.size()); // NOLINT(bugprone-narrowing-conversions)
        *currentVirtual = (*currentVirtual + direction + totalPages) % totalPages;

        if (*currentVirtual < 2) {
            m_tabWidget->setCurrentIndex(*currentVirtual);
        } else {
            int extraIdx = *currentVirtual - 2;
            switchDynamicTab(m_extraPages[extraIdx].second, m_extraPages[extraIdx].first);
        }
        m_calcPage->setFocus();
    };

    // Keep currentVirtual in sync when user clicks tabs manually
    connect(m_tabWidget, &QTabWidget::currentChanged, this, [this, currentVirtual](int index) {
        if (index < 2) {
            *currentVirtual = index;
        } else if (index == 2) {
            // Find which extra page is currently shown
            QWidget* current = m_tabWidget->widget(2);
            for (int i = 0; i < m_extraPages.size(); ++i) {
                if (m_extraPages[i].second == current) {
                    *currentVirtual = 2 + i;
                    break;
                }
            }
        }
    });

    auto* prevTab = new QShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_Left), this);
    connect(prevTab, &QShortcut::activated, this, [navigateTab]() { navigateTab(-1); });
    auto* nextTab = new QShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_Right), this);
    connect(nextTab, &QShortcut::activated, this, [navigateTab]() { navigateTab(1); });

    // Return focus to the calculator when clicking on empty area
    connect(qApp, &QApplication::focusChanged, this, [this](QWidget* /*old*/, QWidget* now) {
    // If focus goes to the tab bar or to no widget, return focus to calculator page
    if (isActiveWindow()) {
        if (!now || qobject_cast<QTabBar*>(now) ||
            now == m_tabWidget || now == m_onTopBtn) {
            QTimer::singleShot(0, this, [this]() {
                if (m_tabWidget->currentIndex() == 0) {
                    m_calcPage->setFocus();
                }
            });
        }
    }
});

    // Auto-update check
    auto* updater = new Rheno::Core::Updater(this);
    connect(updater, &Rheno::Core::Updater::updateAvailable, this, [this](const QString& version, const QString& releaseUrl) {
        updateStatusBar(version, releaseUrl);
    });
    QTimer::singleShot(1500, updater, &Rheno::Core::Updater::checkForUpdate);
}

MainWindow::~MainWindow() = default;

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
    settings.setValue("lastWindowPos", pos());

    // Save the name of the currently shown dynamic tab
    QWidget* dynWidget = m_tabWidget->widget(2);
    for (const auto& [name, page] : m_extraPages) {
        if (page == dynWidget) {
            settings.setValue("dynamicTab", name);
            break;
        }
    }
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

void MainWindow::moveToMousePosition() {
    QPoint mousePos = QCursor::pos();

    QScreen *currentScreen = QGuiApplication::screenAt(mousePos);

    // If no current screen is detected (eg. VM environment), no movement to mouse position
    if (currentScreen) {
        int x = mousePos.x() - (this->width() / 2);
        int y = mousePos.y() - (this->height() / 2);

        // Keep the window inside the visible screen bounds (so it doesn't clip out)
        QRect screenGeometry = currentScreen->geometry();
        if (x < screenGeometry.left()) x = screenGeometry.left();
        if (y < screenGeometry.top()) y = screenGeometry.top();
        if (x + this->width() > screenGeometry.right()) x = screenGeometry.right() - this->width();
        if (y + this->height() > screenGeometry.bottom()) y = screenGeometry.bottom() - this->height();

        this->move(x, y);
    }
}

void MainWindow::centerOnMouseScreen() {
    QPoint mousePos = QCursor::pos();
    QScreen* currentScreen = QGuiApplication::screenAt(mousePos);

    if (currentScreen) {
        QRect screenGeometry = currentScreen->availableGeometry();
        int x = screenGeometry.left() + (screenGeometry.width() - this->width()) / 2;
        int y = screenGeometry.top() + (screenGeometry.height() - this->height()) / 2;
        this->move(x, y);
    }
}

void MainWindow::setWindowPosition() {
    switch (m_windowStartPosition) {
        case WindowStartPosition::LastPosition:
            // Do nothing - restoreWindowGeometry() has already been executed
            break;

        case WindowStartPosition::CenterOnScreen:
            centerOnMouseScreen();
            break;

        case WindowStartPosition::AtMousePosition:
            moveToMousePosition();
            break;
    }
}

void MainWindow::setupUI() {
    setMinimumWidth(310);

    m_tabWidget = new FixedTabWidget(this);
    m_tabWidget->setDocumentMode(true);

    m_calcPage = new CalculatorPage(this);
    m_basePage = new BaseConverterPage(this);
    m_unitPage = new UnitConverterPage(this);
    m_networkPage = new NetworkPage(this);
    m_crcHashPage = new CrcHashPage(this);
    m_colorPage = new ColorPage(this);
    m_financePage = new FinancePage(this);
    m_floatPage = new FloatPage(this);
    m_electronicsPage = new ElectronicsPage(this);
    m_settingsPage = new SettingsPage(this, this);

    // Hide pages not initially in the tab widget so they don't appear as floating children
    m_networkPage->hide();
    m_crcHashPage->hide();
    m_colorPage->hide();
    m_financePage->hide();
    m_floatPage->hide();
    m_electronicsPage->hide();
    m_settingsPage->hide();

    m_tabWidget->addTab(m_calcPage, "Calc");       // index 0 - fixed
    m_tabWidget->addTab(m_basePage, "Base");       // index 1 - fixed
    m_tabWidget->addTab(m_unitPage, "Unit");       // index 2 - dynamic slot (default: Unit)
    m_tabWidget->addTab(new QWidget(this), "▾");   // index 3 - "More" menu trigger

    // Prevent tab bar from stretching tabs to fill the width
    m_tabWidget->tabBar()->setExpanding(false);

    // Build the list of extra pages available via "More"
    m_extraPages = {
        {"Unit",     m_unitPage},
        {"Float",    m_floatPage},
        {"CRC/Hash", m_crcHashPage},
        {"Finance",  m_financePage},
        {"Color",    m_colorPage},
        {"IP-Address",  m_networkPage},
        {"Electronics", m_electronicsPage},
        {"Settings", m_settingsPage},
    };

    // Restore last dynamic tab from settings
    {
        QSettings settings("RhenoCalc", "RhenoCalc");
        QString lastDyn = settings.value("dynamicTab", "Unit").toString();
        for (const auto& [name, page] : m_extraPages) {
            if (name == lastDyn && page != m_unitPage) {
                m_tabWidget->removeTab(2);
                m_tabWidget->insertTab(2, page, name);
                break;
            }
        }
    }

    auto previousTab = std::make_shared<int>(0);

    // Build the "More" popup menu
    m_moreMenu = new QMenu(this);
    for (const auto& [name, page] : m_extraPages) {
        m_moreMenu->addAction(name, this, [this, page, name, previousTab]() {
            switchDynamicTab(page, name);
            *previousTab = 2;
        });
    }

    // Intercept click on the "More" tab (index 3): show menu instead of switching
    connect(m_tabWidget, &QTabWidget::tabBarClicked, this, [this, previousTab](int index) {
        if (index == 3) {
            // Show menu below the "More" tab
            QRect tabRect = m_tabWidget->tabBar()->tabRect(3);
            QPoint pos = m_tabWidget->tabBar()->mapToGlobal(tabRect.bottomLeft());
            m_moreMenu->popup(pos);
        } else {
            *previousTab = index;
        }
    });

    // Prevent the "More" tab from actually being selected – return to previous tab
    connect(m_tabWidget, &QTabWidget::currentChanged, this, [this, previousTab](int index) {
        if (index == 3) {
            m_tabWidget->setCurrentIndex(*previousTab);
        }
    });

    // Buttons top right in the tab bar
    m_onTopBtn = new QPushButton(this); // NOLINT(cppcoreguidelines-owning-memory)
    m_onTopBtn->setCheckable(true);
    m_onTopBtn->setFixedSize(32, 26);
    m_onTopBtn->setCursor(Qt::PointingHandCursor);
    m_onTopBtn->setFocusPolicy(Qt::NoFocus);
    m_onTopBtn->setIcon(QIcon(":/symbols/pin_gray.svg"));
    m_onTopBtn->setIconSize(QSize(14, 14));
    connect(m_onTopBtn, &QPushButton::clicked, this, [this]() {
        applyAlwaysOnTop(!m_alwaysOnTop, true);
        m_calcPage->setFocus();
    });

    auto* corner = new QWidget(this);
    auto* cornerLayout = new QHBoxLayout(corner);
    cornerLayout->setContentsMargins(0, 0, 4, 0);
    cornerLayout->setSpacing(4);
    cornerLayout->addWidget(m_onTopBtn);
    m_tabWidget->setCornerWidget(corner, Qt::TopRightCorner);

    // Connect SettingsPage signals
    connect(m_settingsPage, &SettingsPage::themeChanged, this, [this](bool dark) {
        m_isDark = dark;
        applyTheme(dark);
    });
    connect(m_settingsPage, &SettingsPage::alwaysOnTopChanged, this, [this](bool enabled) {
        applyAlwaysOnTop(enabled, true);
    });
    connect(m_settingsPage, &SettingsPage::windowPositionChanged, this, [this](int mode) {
        m_windowStartPosition = static_cast<WindowStartPosition>(mode);
    });

    setCentralWidget(m_tabWidget);

    // Create status bar with clickable label
    m_statusLabel = new QLabel(this);
    m_statusLabel->setOpenExternalLinks(true);
    m_statusLabel->setTextFormat(Qt::RichText);
    m_statusLabel->setContentsMargins(6, 0, 0, 0);
    statusBar()->addWidget(m_statusLabel, 1);
    updateStatusBar();
}

void MainWindow::applyTheme(bool dark) {
    m_isDark = dark;

    // ── Load global stylesheet from QSS file ─────────────────────────────────
    QFile qss(dark ? ":/styles/dark.qss" : ":/styles/light.qss");
    if (qss.open(QFile::ReadOnly)) {
        qApp->setStyleSheet(Rheno::UI::applyQssColors(QString::fromUtf8(qss.readAll()), dark));
        qss.close();
    }

    // ── Fusion palette (fallback for native elements) ─────────────────────────
    qApp->setStyle(QStyleFactory::create("Fusion"));
    qApp->setPalette(Rheno::UI::applicationPalette(dark));

    // ── Update page-specific styles ────────────────────────────────────────────
    m_calcPage->applyTheme(dark);
    m_basePage->applyTheme(dark);
    m_unitPage->applyTheme(dark);
    m_networkPage->applyTheme(dark);
    m_crcHashPage->applyTheme(dark);
    m_colorPage->applyTheme(dark);
    m_financePage->applyTheme(dark);
    m_floatPage->applyTheme(dark);
    m_electronicsPage->applyTheme(dark);
    m_settingsPage->applyTheme(dark);

    // Status Bar
    statusBar()->setStyleSheet(Rheno::UI::statusBarStyle(dark));
    updateStatusBar(); // Refresh link color for current theme


    updateOnTopButton();
}

void MainWindow::applyAlwaysOnTop(bool enabled, bool persist) {
    m_alwaysOnTop = enabled;

    if (persist) {
        QSettings settings("RhenoCalc", "RhenoCalc");
        settings.setValue("alwaysOnTop", m_alwaysOnTop);
    }

    // Toggle the flag and re-show the window.
    const bool wasVisible = isVisible();
    const QRect geo = geometry();

    Qt::WindowFlags flags = windowFlags();
    if (enabled)
        flags |= Qt::WindowStaysOnTopHint;
    else
        flags &= ~Qt::WindowStaysOnTopHint;

    // setWindowFlags() hides the window internally – save geometry first.
    setWindowFlags(flags);

    if (wasVisible) {
        setGeometry(geo);
        show();
        raise();
        activateWindow();
    }


    updateOnTopButton();
}

void MainWindow::updateOnTopButton() {
    if (!m_onTopBtn)
        return;

    m_onTopBtn->setChecked(m_alwaysOnTop);
    m_onTopBtn->setToolTip(m_alwaysOnTop ? "Always on top: ON" : "Always on top: OFF");

    QString style = Rheno::UI::themeToggleButtonStyle(m_isDark);
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

void MainWindow::switchDynamicTab(QWidget* page, const QString& title) {
    // Replace the widget at index 2 (the dynamic slot)
    QWidget* current = m_tabWidget->widget(2);
    if (current == page) {
        // Already showing this page, just focus it
        m_tabWidget->setCurrentIndex(2);
        return;
    }

    // Remove old dynamic tab and insert new one at same position
    m_tabWidget->removeTab(2);
    m_tabWidget->insertTab(2, page, title);
    m_tabWidget->setCurrentIndex(2);
}

void MainWindow::updateStatusBar(const QString& updateVersion, const QString& releaseUrl) {
    if (!m_statusLabel) return;

    // Store values if provided (for theme refresh)
    if (!updateVersion.isEmpty()) {
        m_updateVersion = updateVersion;
        m_updateUrl = releaseUrl;
    }

    QString linkColor = m_isDark ? "#6eb5ff" : "#0066cc";
    QString statusText;

    if (m_updateVersion.isEmpty()) {
        // Normal status - no update available
        statusText = QString(
        "<table width=\"100%\" style=\"border-collapse: collapse;\">"
            "  <tr>"
            "    <td style=\"text-align: left; width: 33%;\"> RhenoCalc  |  Embedded Engineering Toolbox</td>"
            "    <td style=\"text-align: right; width: 33%;\"> | %1</td>"
            "  </tr>"
            "</table>"
            )
            .arg(APP_VERSION_STRING);
    } else {
        // Update available - show clickable link
        statusText = QString(
            "<table width=\"100%\" style=\"border-collapse: collapse;\">"
            "  <tr>"
            "    <td style=\"text-align: left; width: 33%;\"> RhenoCalc  |  <a href=\"%2\" style=\"color:%3;\">Update %1 available</a></td>"
            "    <td style=\"text-align: right; width: 33%;\"> | %4</td>"
            "  </tr>"
            "</table>"
        ).arg(m_updateVersion, m_updateUrl, linkColor, APP_VERSION_STRING);
    }
    m_statusLabel->setText(statusText);
}
