#include "nettoolspage.h"
#include "core/nettools.h"
#include "core/traceroute.h"
#include "ui/themecolors.h"

#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QTextEdit>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QRegularExpression>

NetToolsPage::NetToolsPage(QWidget* parent) : QWidget(parent) {
    setupUI();
    applyTheme(true);
}

NetToolsPage::~NetToolsPage() {
    stopPing();
    stopPortScan();
    stopTraceroute();
}

void NetToolsPage::setupUI() {
    auto* root = new QVBoxLayout(this);
    root->setSizeConstraint(QLayout::SetNoConstraint);
    root->setSpacing(0);
    root->setContentsMargins(0, 0, 0, 0);

    auto* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    auto* content = new QWidget(scroll);
    auto* contentLayout = new QVBoxLayout(content);
    contentLayout->setSizeConstraint(QLayout::SetNoConstraint);
    contentLayout->setSpacing(8);
    contentLayout->setContentsMargins(14, 12, 14, 12);

    m_titleLabel = new QLabel("Network Tools", this);
    contentLayout->addWidget(m_titleLabel);

    // ═══════════════════════════════════════════════════════════════════════════
    // Ping Tool
    // ═══════════════════════════════════════════════════════════════════════════
    auto* pingGroup = new QGroupBox("Ping", this);
    auto* pingLayout = new QVBoxLayout(pingGroup);

    auto* pingHostLayout = new QHBoxLayout();
    pingHostLayout->addWidget(new QLabel("Host:", this));
    m_pingHost = new QLineEdit("8.8.8.8", this);
    m_pingHost->setPlaceholderText("IP or hostname");
    pingHostLayout->addWidget(m_pingHost, 1);
    pingLayout->addLayout(pingHostLayout);

    auto* pingControlLayout = new QHBoxLayout();
    pingControlLayout->addWidget(new QLabel("Count:", this));
    m_pingCount = new QLineEdit("4", this);
    m_pingCount->setFixedWidth(50);
    m_pingCount->setPlaceholderText("1-100");
    pingControlLayout->addWidget(m_pingCount);
    pingControlLayout->addStretch();

    m_pingStartBtn = new QPushButton("Start", this);
    m_pingStartBtn->setFixedWidth(60);
    pingControlLayout->addWidget(m_pingStartBtn);

    m_pingStopBtn = new QPushButton("Stop", this);
    m_pingStopBtn->setFixedWidth(60);
    m_pingStopBtn->setEnabled(false);
    pingControlLayout->addWidget(m_pingStopBtn);
    pingLayout->addLayout(pingControlLayout);

    m_pingOutput = new QTextEdit(this);
    m_pingOutput->setReadOnly(true);
    m_pingOutput->setMinimumHeight(80);
    m_pingOutput->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_pingOutput->setFont(QFont("Consolas", 9));
    pingLayout->addWidget(m_pingOutput);

    contentLayout->addWidget(pingGroup);

    // ═══════════════════════════════════════════════════════════════════════════
    // Port Scanner
    // ═══════════════════════════════════════════════════════════════════════════
    auto* scanGroup = new QGroupBox("Port Scanner", this);
    auto* scanLayout = new QVBoxLayout(scanGroup);

    auto* scanHostLayout = new QHBoxLayout();
    scanHostLayout->addWidget(new QLabel("Host:", this));
    m_scanHost = new QLineEdit("127.0.0.1", this);
    m_scanHost->setPlaceholderText("IP or hostname");
    scanHostLayout->addWidget(m_scanHost, 1);
    scanLayout->addLayout(scanHostLayout);

    auto* scanControlLayout = new QHBoxLayout();
    scanControlLayout->addWidget(new QLabel("Ports:", this));
    m_scanPorts = new QLineEdit("21-25,80,443,3389,8080", this);
    m_scanPorts->setPlaceholderText("e.g. 80,443 or 1-1000");
    scanControlLayout->addWidget(m_scanPorts, 1);

    m_scanStartBtn = new QPushButton("Scan", this);
    m_scanStartBtn->setFixedWidth(60);
    scanControlLayout->addWidget(m_scanStartBtn);

    m_scanStopBtn = new QPushButton("Stop", this);
    m_scanStopBtn->setFixedWidth(60);
    m_scanStopBtn->setEnabled(false);
    scanControlLayout->addWidget(m_scanStopBtn);
    scanLayout->addLayout(scanControlLayout);

    m_scanProgress = new QProgressBar(this);
    m_scanProgress->setRange(0, 100);
    m_scanProgress->setValue(0);
    m_scanProgress->setTextVisible(true);
    scanLayout->addWidget(m_scanProgress);

    m_scanOutput = new QTextEdit(this);
    m_scanOutput->setReadOnly(true);
    m_scanOutput->setMinimumHeight(80);
    m_scanOutput->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_scanOutput->setFont(QFont("Consolas", 9));
    scanLayout->addWidget(m_scanOutput);

    contentLayout->addWidget(scanGroup);

    // ═══════════════════════════════════════════════════════════════════════════
    // Traceroute
    // ═══════════════════════════════════════════════════════════════════════════
    auto* traceGroup = new QGroupBox("Traceroute", this);
    auto* traceLayout = new QVBoxLayout(traceGroup);

    auto* traceInputLayout = new QHBoxLayout();
    traceInputLayout->addWidget(new QLabel("Host:", this));
    m_traceHost = new QLineEdit("8.8.8.8", this);
    m_traceHost->setPlaceholderText("IP or hostname");
    traceInputLayout->addWidget(m_traceHost, 1);

    m_traceStartBtn = new QPushButton("Trace", this);
    m_traceStartBtn->setFixedWidth(60);
    traceInputLayout->addWidget(m_traceStartBtn);

    m_traceStopBtn = new QPushButton("Stop", this);
    m_traceStopBtn->setFixedWidth(60);
    m_traceStopBtn->setEnabled(false);
    traceInputLayout->addWidget(m_traceStopBtn);

    traceLayout->addLayout(traceInputLayout);

    m_traceOutput = new QTextEdit(this);
    m_traceOutput->setReadOnly(true);
    m_traceOutput->setMinimumHeight(80);
    m_traceOutput->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_traceOutput->setFont(QFont("Consolas", 9));
    traceLayout->addWidget(m_traceOutput);

    contentLayout->addWidget(traceGroup);

    scroll->setWidget(content);
    root->addWidget(scroll);

    // Connect signals
    connect(m_pingStartBtn, &QPushButton::clicked, this, &NetToolsPage::startPing);
    connect(m_pingStopBtn, &QPushButton::clicked, this, &NetToolsPage::stopPing);

    connect(m_scanStartBtn, &QPushButton::clicked, this, &NetToolsPage::startPortScan);
    connect(m_scanStopBtn, &QPushButton::clicked, this, &NetToolsPage::stopPortScan);

    connect(m_traceStartBtn, &QPushButton::clicked, this, &NetToolsPage::startTraceroute);
    connect(m_traceStopBtn, &QPushButton::clicked, this, &NetToolsPage::stopTraceroute);

    // Initialize timer for port scanning
    m_scanTimer = new QTimer(this);
    m_scanTimer->setSingleShot(true);
    m_scanTimer->setInterval(500); // timeout per port

    connect(m_scanTimer, &QTimer::timeout, this, &NetToolsPage::onPortTimeout);
}

void NetToolsPage::appendOutput(QTextEdit* output, const QString& text, const QString& color) {
    if (color.isEmpty()) {
        output->append(text);
    } else {
        output->append(QString("<span style='color:%1'>%2</span>").arg(color, text.toHtmlEscaped()));
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
// Ping
// ═══════════════════════════════════════════════════════════════════════════════
void NetToolsPage::startPing() {
    QString host = m_pingHost->text().trimmed();
    if (host.isEmpty()) return;

    m_pingOutput->clear();
    m_pingStartBtn->setEnabled(false);
    m_pingStopBtn->setEnabled(true);

    m_pingProcess = new QProcess(this);
    connect(m_pingProcess, &QProcess::readyReadStandardOutput, this, &NetToolsPage::onPingOutput);
    connect(m_pingProcess, &QProcess::readyReadStandardError, this, &NetToolsPage::onPingOutput);
    connect(m_pingProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &NetToolsPage::onPingFinished);

#ifdef Q_OS_WIN
    m_pingProcess->start("ping", {"-n", m_pingCount->text().trimmed(), host});
#else
    m_pingProcess->start("ping", {"-c", m_pingCount->text().trimmed(), host});
#endif

    appendOutput(m_pingOutput, QString("Pinging %1...").arg(host), "#888");
}

void NetToolsPage::stopPing() {
    if (m_pingProcess) {
        m_pingProcess->kill();
        m_pingProcess->deleteLater();
        m_pingProcess = nullptr;
    }
    m_pingStartBtn->setEnabled(true);
    m_pingStopBtn->setEnabled(false);
}

void NetToolsPage::onPingOutput() {
    if (!m_pingProcess) return;
    QString output = QString::fromLocal8Bit(m_pingProcess->readAllStandardOutput());
    output += QString::fromLocal8Bit(m_pingProcess->readAllStandardError());

    for (const QString& line : output.split('\n', Qt::SkipEmptyParts)) {
        const QString trimmed = line.trimmed();
        if (trimmed.isEmpty()) continue;
        const auto styled = Rheno::Core::classifyPingLine(trimmed);
        appendOutput(m_pingOutput, styled.text, styled.color);
    }
}

void NetToolsPage::onPingFinished(int exitCode, QProcess::ExitStatus /*status*/) {
    appendOutput(m_pingOutput, QString("\nPing finished (exit code: %1)").arg(exitCode), "#888");
    stopPing();
}

// ═══════════════════════════════════════════════════════════════════════════════
// Port Scanner
// ═══════════════════════════════════════════════════════════════════════════════
void NetToolsPage::startPortScan() {
    QString host = m_scanHost->text().trimmed();
    QString portsStr = m_scanPorts->text().trimmed();
    if (host.isEmpty() || portsStr.isEmpty()) return;

    m_portsToScan = Rheno::Core::parsePorts(portsStr);
    if (m_portsToScan.isEmpty()) {
        appendOutput(m_scanOutput, "No valid ports specified", "#e74c3c");
        return;
    }

    m_scanOutput->clear();
    m_scanStartBtn->setEnabled(false);
    m_scanStopBtn->setEnabled(true);

    appendOutput(m_scanOutput, QString("Resolving %1...").arg(host), "#888");

    // Resolve hostname first
    QHostInfo::lookupHost(host, this, [this, host](const QHostInfo& info) {
        if (info.error() != QHostInfo::NoError || info.addresses().isEmpty()) {
            appendOutput(m_scanOutput, QString("Failed to resolve host: %1").arg(info.errorString()), "#e74c3c");
            m_scanStartBtn->setEnabled(true);
            m_scanStopBtn->setEnabled(false);
            return;
        }

        m_resolvedScanAddress = info.addresses().first();
        m_currentPortIndex = 0;
        m_openPorts = 0;
        m_scanActive = true;

        m_scanProgress->setRange(0, m_portsToScan.size());
        m_scanProgress->setValue(0);

        appendOutput(m_scanOutput, QString("Scanning %1 (%2) - %3 ports...")
            .arg(host)
            .arg(m_resolvedScanAddress.toString())
            .arg(m_portsToScan.size()), "#888");

        scanNextPort();
    });
}

void NetToolsPage::stopPortScan() {
    m_scanActive = false;
    m_scanTimer->stop();
    if (m_scanSocket) {
        m_scanSocket->disconnect();
        m_scanSocket->abort();
        m_scanSocket->deleteLater();
        m_scanSocket = nullptr;
    }
    m_portsToScan.clear();
    m_currentPortIndex = 0;

    m_scanStartBtn->setEnabled(true);
    m_scanStopBtn->setEnabled(false);
}

void NetToolsPage::scanNextPort() {
    if (!m_scanActive) return;

    if (m_currentPortIndex >= m_portsToScan.size()) {
        // Scan complete
        appendOutput(m_scanOutput, QString("\nScan complete. %1 open port(s) found.").arg(m_openPorts), "#888");
        stopPortScan();
        return;
    }

    int port = m_portsToScan[m_currentPortIndex];
    m_scanProgress->setValue(m_currentPortIndex + 1);

    // Clean up previous socket
    if (m_scanSocket) {
        m_scanSocket->disconnect();
        m_scanSocket->abort();
        m_scanSocket->deleteLater();
        m_scanSocket = nullptr;
    }

    // Create a new socket for each port to avoid state issues
    m_scanSocket = new QTcpSocket(this);
    connect(m_scanSocket, &QTcpSocket::connected, this, &NetToolsPage::onPortConnected);
    connect(m_scanSocket, &QAbstractSocket::errorOccurred, this, &NetToolsPage::onPortError);

    m_scanSocket->connectToHost(m_resolvedScanAddress, static_cast<quint16>(port));
    m_scanTimer->start();
}

void NetToolsPage::onPortConnected() {
    if (!m_scanActive) return;
    m_scanTimer->stop();

    if (m_currentPortIndex >= m_portsToScan.size()) return;

    int port = m_portsToScan[m_currentPortIndex];
    m_openPorts++;

    QString service = Rheno::Core::serviceNameForPort(port);

    QString portInfo = QString::number(port);
    if (!service.isEmpty()) portInfo += " (" + service + ")";

    appendOutput(m_scanOutput, QString("  Port %1 - OPEN").arg(portInfo), "#2ecc71");

    if (m_scanSocket) {
        m_scanSocket->abort();
    }
    m_currentPortIndex++;

    // Use single-shot timer to avoid deep recursion
    QTimer::singleShot(0, this, &NetToolsPage::scanNextPort);
}

void NetToolsPage::onPortError(QAbstractSocket::SocketError /*error*/) {
    if (!m_scanActive) return;
    m_scanTimer->stop();

    if (m_scanSocket) {
        m_scanSocket->abort();
    }
    m_currentPortIndex++;

    // Use single-shot timer to avoid deep recursion
    QTimer::singleShot(0, this, &NetToolsPage::scanNextPort);
}

void NetToolsPage::onPortTimeout() {
    if (!m_scanActive) return;

    if (m_scanSocket) {
        m_scanSocket->disconnect();
        m_scanSocket->abort();
    }
    m_currentPortIndex++;

    // Use single-shot timer to avoid deep recursion
    QTimer::singleShot(0, this, &NetToolsPage::scanNextPort);
}

// ═══════════════════════════════════════════════════════════════════════════════
// Traceroute
// ═══════════════════════════════════════════════════════════════════════════════
void NetToolsPage::startTraceroute() {
    QString host = m_traceHost->text().trimmed();
    if (host.isEmpty()) return;

    m_traceOutput->clear();
    m_traceStartBtn->setEnabled(false);
    m_traceStopBtn->setEnabled(true);

    if (!m_traceroute) {
        m_traceroute = new Traceroute(this);
        connect(m_traceroute, &Traceroute::hopResult, this, &NetToolsPage::onTraceHop);
        connect(m_traceroute, &Traceroute::finished, this, &NetToolsPage::onTraceFinished);
        connect(m_traceroute, &Traceroute::error, this, &NetToolsPage::onTraceError);
    }

    appendOutput(m_traceOutput, QString("Tracing route to %1...").arg(host), "#888");
    m_traceroute->start(host, 30, 3000);
}

void NetToolsPage::stopTraceroute() {
    if (m_traceroute) {
        m_traceroute->stop();
    }
    m_traceStartBtn->setEnabled(true);
    m_traceStopBtn->setEnabled(false);
}

void NetToolsPage::onTraceHop(const TraceHop& hop) {
    const auto styled = Rheno::Core::formatTraceHopLine(hop.hop, hop.rtt, hop.address);
    appendOutput(m_traceOutput, styled.text, styled.color);
}

void NetToolsPage::onTraceFinished(bool success, const QString& message) {
    Q_UNUSED(success)
    appendOutput(m_traceOutput, QString("\n%1").arg(message), "#888");
    stopTraceroute();
}

void NetToolsPage::onTraceError(const QString& message) {
    appendOutput(m_traceOutput, message, "#e74c3c");
}

void NetToolsPage::applyTheme(bool dark) {
    m_isDark = dark;

    const QString grpS = Rheno::UI::unitGroupStyle(dark);
    const QString fldS = Rheno::UI::unitFieldStyle(dark);
    const QString ttlS = Rheno::UI::unitTitleStyle(dark);
    const QString frmS = Rheno::UI::unitFormulaStyle(dark);

    m_titleLabel->setStyleSheet(ttlS);

    for (auto* box : findChildren<QGroupBox*>())
        box->setStyleSheet(grpS);

    for (auto* edit : findChildren<QLineEdit*>())
        edit->setStyleSheet(fldS);

    // Style text outputs - matching theme colors
    QString outputStyle = dark
        ? "QTextEdit { background-color: #444444; color: #f0f0f0; border: 1px solid #666666; border-radius: 4px; }"
        : "QTextEdit { background-color: #ffffff; color: #333333; border: 1px solid #cccccc; border-radius: 4px; }";

    m_pingOutput->setStyleSheet(outputStyle);
    m_scanOutput->setStyleSheet(outputStyle);
    m_traceOutput->setStyleSheet(outputStyle);

    // Style buttons - matching theme colors
    QString btnStyle = dark
        ? "QPushButton { background-color: #505050; color: #f0f0f0; border: 1px solid #666666; border-radius: 4px; padding: 4px 8px; }"
          "QPushButton:hover { background-color: #686868; }"
          "QPushButton:disabled { background-color: #3a3a3a; color: #666666; }"
        : "QPushButton { background-color: #e8eaf5; color: #333333; border: 1px solid #cccccc; border-radius: 4px; padding: 4px 8px; }"
          "QPushButton:hover { background-color: #dde3f5; }"
          "QPushButton:disabled { background-color: #f4f6fb; color: #aaaaaa; }";

    for (auto* btn : findChildren<QPushButton*>())
        btn->setStyleSheet(btnStyle);

    for (auto* label : findChildren<QLabel*>())
        if (label != m_titleLabel)
            label->setStyleSheet(frmS);

    m_titleLabel->setStyleSheet(ttlS);
}

