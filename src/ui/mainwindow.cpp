#include "mainwindow.h"
#include "pages/calculatorpage.h"
#include "pages/baseconverterpage.h"
#include "pages/unitconverterpage.h"
#include "pages/networkpage.h"
#include "pages/crchashpage.h"
#include "pages/colorpage.h"
#include "pages/financepage.h"
#include "pages/floatpage.h"
#include "themecolors.h"
#include "info.h"
#include "core/updater.h"
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
#include <QMessageBox>
#include <QProcess>
#include <QDir>
#include <QProgressDialog>
#include <QThread>
#include <QTimer>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_tabWidget(nullptr)
    , m_calcPage(nullptr)
    , m_basePage(nullptr)
    , m_unitPage(nullptr)
    , m_networkPage(nullptr)
    , m_crcHashPage(nullptr)
    , m_colorPage(nullptr)
    , m_financePage(nullptr)
    , m_floatPage(nullptr)
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
        if (!now && isActiveWindow())
            m_calcPage->setFocus();
    });

    // Auto-update check
    auto* updater = new Updater(this);
    connect(updater, &Updater::updateAvailable, this, [this, updater](const QString& version, const QString& url) {
        QMessageBox msgBox(QMessageBox::Question, "Update Available",
            QString("A new version %1 is available.\nDo you want to update now?").arg(version),
            QMessageBox::Yes | QMessageBox::No, this);
        msgBox.setFixedWidth(this->width());
        auto result = msgBox.exec();

        if (result == QMessageBox::Yes) {
            // Create progress dialog for download
            auto* progress = new QProgressDialog("Downloading update...", "Cancel", 0, 100, this);
            progress->setWindowModality(Qt::WindowModal);
            progress->setMinimumDuration(0);
            progress->setFixedWidth(this->width());
            progress->setValue(0);
            progress->show();

            connect(updater, &Updater::downloadProgress, this, [progress](qint64 received, qint64 total) {
                if (total > 0) {
                    progress->setMaximum(static_cast<int>(total));
                    progress->setValue(static_cast<int>(received));
                    progress->setLabelText(QString("Downloading... %1 / %2 KB")
                        .arg(received / 1024).arg(total / 1024));
                } else {
                    progress->setMaximum(0); // Indeterminate
                    progress->setLabelText("Downloading...");
                }
            });

            connect(updater, &Updater::downloadFinished, this, [this, updater, progress](const QString& zipPath) {
                progress->close();
                delete progress;

                // Cleanup updater resources BEFORE starting batch
                updater->cleanup();

                // Start batch script - it will extract, copy, and restart
                QString appDir = QDir::toNativeSeparators(QApplication::applicationDirPath());
                QString script = QDir::toNativeSeparators(appDir + "\\update.bat");
                QString appExe = QDir::toNativeSeparators(QApplication::applicationFilePath());
                QString nativeZipPath = QDir::toNativeSeparators(zipPath);

                bool started = QProcess::startDetached("cmd.exe",
                    QStringList{"/c", "call", script, nativeZipPath, appDir, appExe}, appDir);

                if (!started) {
                    QMessageBox errorBox(QMessageBox::Warning, "Update Error",
                        "Could not start the update script.\nPlease update manually.",
                        QMessageBox::Ok, this);
                    errorBox.setFixedWidth(this->width());
                    errorBox.exec();
                    return;
                }

                // Give batch time to start, then quit
                QThread::msleep(200);
                QApplication::quit();
            });

            connect(updater, &Updater::updateError, this, [this, progress](const QString& error) {
                progress->close();
                delete progress;
                QMessageBox errorBox(QMessageBox::Warning, "Update Error", error, QMessageBox::Ok, this);
                errorBox.setFixedWidth(this->width());
                errorBox.exec();
            });

            connect(progress, &QProgressDialog::canceled, this, [updater]() {
                updater->cleanup();
            });

            updater->downloadUpdate(url);
        }
    });
    QTimer::singleShot(1500, updater, &Updater::checkForUpdate);
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

void MainWindow::setupUI() {
    m_tabWidget = new FixedTabWidget(this);

    m_calcPage = new CalculatorPage(this);
    m_basePage = new BaseConverterPage(this);
    m_unitPage = new UnitConverterPage(this);
    m_networkPage = new NetworkPage(this);
    m_crcHashPage = new CrcHashPage(this);
    m_colorPage = new ColorPage(this);
    m_financePage = new FinancePage(this);
    m_floatPage = new FloatPage(this);

    // Hide pages not initially in the tab widget so they don't appear as floating children
    m_networkPage->hide();
    m_crcHashPage->hide();
    m_colorPage->hide();
    m_financePage->hide();
    m_floatPage->hide();

    m_tabWidget->addTab(m_calcPage, "Calc");       // index 0 - fixed
    m_tabWidget->addTab(m_basePage, "Base");       // index 1 - fixed
    m_tabWidget->addTab(m_unitPage, "Unit");       // index 2 - dynamic slot (default: Unit)
    m_tabWidget->addTab(new QWidget(this), "▾");   // index 3 - "More" menu trigger

    // Prevent tab bar from stretching tabs to fill the width
    m_tabWidget->tabBar()->setExpanding(false);

    // Build the list of extra pages available via "More"
    m_extraPages = {
        {"Unit",     m_unitPage},
        {"Network",  m_networkPage},
        {"CRC/Hash", m_crcHashPage},
        {"Color",    m_colorPage},
        {"Finance",  m_financePage},
        {"Float",    m_floatPage},
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

    // Build the "More" popup menu
    m_moreMenu = new QMenu(this);
    for (const auto& [name, page] : m_extraPages) {
        m_moreMenu->addAction(name, this, [this, page, name]() {
            switchDynamicTab(page, name);
        });
    }

    // Intercept click on the "More" tab (index 3): show menu instead of switching
    auto previousTab = std::make_shared<int>(0);
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
    m_onTopBtn->setIcon(QIcon(":/icons/pin_gray.svg"));
    m_onTopBtn->setIconSize(QSize(14, 14));
    connect(m_onTopBtn, &QPushButton::clicked, this, [this]() {
        applyAlwaysOnTop(!m_alwaysOnTop, true);
        m_calcPage->setFocus();
    });


    m_themeBtn = new QPushButton("☀", this); // NOLINT(cppcoreguidelines-owning-memory)
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
    statusBar()->showMessage(QString("RhenoCalc  |  Embedded Engineering Toolbox  |  ") + APP_VERSION_STRING);
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
    m_crcHashPage->applyTheme(dark);
    m_colorPage->applyTheme(dark);
    m_financePage->applyTheme(dark);
    m_floatPage->applyTheme(dark);

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
