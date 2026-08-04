#include "settingspage.h"
#include "ui/mainwindow.h"
#include "ui/themecolors.h"
#include "core/updater.h"
#include "info.h"
#include <QVBoxLayout>
#include <QGridLayout>
#include <QScrollArea>
#include <QSettings>
#include <QSizePolicy>
#include <QTimer>

SettingsPage::SettingsPage(MainWindow* mainWindow, QWidget* parent)
    : QWidget(parent), m_mainWindow(mainWindow)
{
    setupUI();

    // Auto-update check
    auto* updater = new Rheno::Core::Updater(this);
    connect(updater, &Rheno::Core::Updater::updateAvailable, this, [this](const QString& version, const QString& releaseUrl) {
        updateStatusBar(version, releaseUrl);
    });
    QTimer::singleShot(1500, updater, &Rheno::Core::Updater::checkForUpdate);
}

void SettingsPage::setupUI() {
    setMinimumSize(0, 0);
    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->setSpacing(0);

    auto* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setFrameShape(QFrame::NoFrame);

    auto* content = new QWidget(scroll);
    content->setMinimumSize(0, 0);
    content->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);

    auto* root = new QVBoxLayout(content);
    root->setSizeConstraint(QLayout::SetNoConstraint);
    root->setSpacing(16);
    root->setContentsMargins(16, 16, 16, 16);

    // ── Appearance Settings ──────────────────────────────────────────────────
    m_appearanceGroup = new QGroupBox("Appearance", this);
    m_appearanceGroup->setStyleSheet(Rheno::UI::baseGroupStyle(true));
    auto* appearLayout = new QGridLayout(m_appearanceGroup);
    appearLayout->setSpacing(12);

    auto* themeLabel = new QLabel("Theme:", this);
    themeLabel->setStyleSheet("font-size:13px;");

    m_themeBtn = new QPushButton("☀", this);
    m_themeBtn->setFixedSize(80, 32);
    m_themeBtn->setCursor(Qt::PointingHandCursor);
    m_themeBtn->setFocusPolicy(Qt::NoFocus);

    QSettings settings("RhenoCalc", "RhenoCalc");
    m_isDark = settings.value("darkTheme", true).toBool();

    connect(m_themeBtn, &QPushButton::clicked, this, [this]() {
        m_isDark = !m_isDark;
        emit themeChanged(m_isDark);

        QSettings settings("RhenoCalc", "RhenoCalc");
        settings.setValue("darkTheme", m_isDark);
    });

    appearLayout->addWidget(themeLabel, 0, 0);
    appearLayout->addWidget(m_themeBtn, 0, 1, Qt::AlignLeft);

    root->addWidget(m_appearanceGroup);

    // ── Window Settings ──────────────────────────────────────────────────────
    m_windowGroup = new QGroupBox("Window", this);
    m_windowGroup->setStyleSheet(Rheno::UI::baseGroupStyle(true));
    auto* windowLayout = new QGridLayout(m_windowGroup);
    windowLayout->setSpacing(12);

    // Always On Top
    m_closeWithEscCheck = new QCheckBox("Close with ESC", this);
    m_closeWithEscCheck->setStyleSheet("font-size:13px;");
    bool closeWithEscCheck = settings.value("closeWithEscCheck", true).toBool();
    m_closeWithEscCheck->setChecked(closeWithEscCheck);

    connect(m_closeWithEscCheck, &QCheckBox::toggled, this, [this](bool checked) {
        emit closeWithEscChanged(checked);

        QSettings settings("RhenoCalc", "RhenoCalc");
        settings.setValue("closeWithEscCheck", checked);
    });

    windowLayout->addWidget(m_closeWithEscCheck, 0, 0, 1, 2);

    // Restore Tab Index on start
    m_restoreTabIndexCheck = new QCheckBox("Restore Tab Index", this);
    m_restoreTabIndexCheck->setStyleSheet("font-size:13px;");
    bool restoreTabIndexCheck = settings.value("restoreTabIndexCheck", false).toBool();
    m_restoreTabIndexCheck->setChecked(restoreTabIndexCheck);

    connect(m_restoreTabIndexCheck, &QCheckBox::toggled, this, [this](bool checked) {
        emit restoreTabIndexChanged(checked);

        QSettings settings("RhenoCalc", "RhenoCalc");
        settings.setValue("restoreTabIndexCheck", checked);
    });

    windowLayout->addWidget(m_restoreTabIndexCheck, 1, 0, 1, 2);

    // Window Start Position
    auto* posLabel = new QLabel("Start position:", this);
    posLabel->setStyleSheet("font-size:13px;");

    m_windowPosCombo = new QComboBox(this);
    m_windowPosCombo->addItems({"Last Position", "Center on Screen", "At Mouse Position"});
    int startPos = settings.value("windowStartPosition", 1).toInt();
    m_windowPosCombo->setCurrentIndex(startPos);

    connect(m_windowPosCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
        emit windowPositionChanged(index);

        QSettings settings("RhenoCalc", "RhenoCalc");
        settings.setValue("windowStartPosition", index);
    });

    windowLayout->addWidget(posLabel, 2, 0);
    windowLayout->addWidget(m_windowPosCombo, 2, 1);

    root->addWidget(m_windowGroup);

    // ── About ────────────────────────────────────────────────────────────────
    m_aboutGroup = new QGroupBox("About", this);
    m_aboutGroup->setStyleSheet(Rheno::UI::baseGroupStyle(true));
    m_aboutLayout = new QVBoxLayout(m_aboutGroup);
    m_aboutLayout->setSpacing(8);

    auto* appName = new QLabel("<b>RhenoCalc</b>", this);
    appName->setStyleSheet("font-size:14px;");
    m_aboutLayout->addWidget(appName);

    m_versionLabel = new QLabel(QString("Version: %1").arg(APP_VERSION_STRING), this);
    m_versionLabel->setStyleSheet("font-size:12px;");
    m_aboutLayout->addWidget(m_versionLabel);

    auto* descLabel = new QLabel("Embedded Engineering Toolbox", this);
    descLabel->setStyleSheet("font-size:12px; color: #888;");
    m_aboutLayout->addWidget(descLabel);

    auto* copyrightLabel = new QLabel("© 2026 Rhenosys GmbH", this);
    copyrightLabel->setStyleSheet("font-size:11px; color: #666;");
    m_aboutLayout->addWidget(copyrightLabel);

    root->addWidget(m_aboutGroup);
    root->addStretch();

    scroll->setWidget(content);
    outer->addWidget(scroll);
}

void SettingsPage::applyTheme(bool dark) {
    m_isDark = dark;

    const QString grpS = Rheno::UI::baseGroupStyle(dark);
    m_appearanceGroup->setStyleSheet(grpS);
    m_windowGroup->setStyleSheet(grpS);
    m_aboutGroup->setStyleSheet(grpS);

    // Update theme button
    if (m_themeBtn) {
        m_themeBtn->setText(dark ? "☀" : "🌙");
        m_themeBtn->setStyleSheet(Rheno::UI::themeToggleButtonStyle(dark));
    }
}

void SettingsPage::updateStatusBar(const QString& updateVersion, const QString& releaseUrl) {

    // Store values if provided (for theme refresh)
    if (!updateVersion.isEmpty()) {
        m_updateVersion = updateVersion;
        m_updateUrl = releaseUrl;
    }

    QString linkColor = m_isDark ? "#6eb5ff" : "#0066cc";
    QString statusText;

    if (!m_updateVersion.isEmpty()) {
        // Update available - show clickable link
        m_updateText = QString(
            "<table width=\"100%\" style=\"border-collapse: collapse;\">"
            "  <tr>"
            "    <td style=\"text-align: left; width: 33%;\">Update: <a href=\"%2\" style=\"color:%3;\">%1 available</a></td>"
            "  </tr>"
            "</table>"
        ).arg(m_updateVersion, m_updateUrl, linkColor, APP_VERSION_STRING);

        m_versionUpdateAvailable = new QLabel(m_updateText, this);
        m_versionUpdateAvailable->setStyleSheet("font-size:12px;");
        m_aboutLayout->insertWidget(2, m_versionUpdateAvailable);
    }
}