#pragma once

#include <QWidget>
#include <QProcess>
#include <QTcpSocket>
#include <QTimer>

class QLabel;
class QLineEdit;
class QPushButton;
class QTextEdit;
class QSpinBox;
class QProgressBar;

class NetToolsPage : public QWidget {
    Q_OBJECT

public:
    explicit NetToolsPage(QWidget* parent = nullptr);
    ~NetToolsPage() override;
    void applyTheme(bool dark);

private slots:
    void startPing();
    void stopPing();
    void onPingOutput();
    void onPingFinished(int exitCode, QProcess::ExitStatus status);

    void startPortScan();
    void stopPortScan();
    void scanNextPort();
    void onPortConnected();
    void onPortError(QAbstractSocket::SocketError error);

    void startTraceroute();
    void stopTraceroute();
    void onTracerouteOutput();
    void onTracerouteFinished(int exitCode, QProcess::ExitStatus status);

private:
    void setupUI();
    void appendOutput(QTextEdit* output, const QString& text, const QString& color = "");

    QLabel* m_titleLabel = nullptr;

    // Ping
    QLineEdit* m_pingHost = nullptr;
    QSpinBox* m_pingCount = nullptr;
    QPushButton* m_pingStartBtn = nullptr;
    QPushButton* m_pingStopBtn = nullptr;
    QTextEdit* m_pingOutput = nullptr;
    QProcess* m_pingProcess = nullptr;

    // Port Scanner
    QLineEdit* m_scanHost = nullptr;
    QLineEdit* m_scanPorts = nullptr;
    QPushButton* m_scanStartBtn = nullptr;
    QPushButton* m_scanStopBtn = nullptr;
    QProgressBar* m_scanProgress = nullptr;
    QTextEdit* m_scanOutput = nullptr;
    QTcpSocket* m_scanSocket = nullptr;
    QTimer* m_scanTimer = nullptr;
    QList<int> m_portsToScan;
    int m_currentPortIndex = 0;
    int m_openPorts = 0;
    QString m_scanTargetHost;

    // Traceroute
    QLineEdit* m_traceHost = nullptr;
    QPushButton* m_traceStartBtn = nullptr;
    QPushButton* m_traceStopBtn = nullptr;
    QTextEdit* m_traceOutput = nullptr;
    QProcess* m_traceProcess = nullptr;

    bool m_isDark = true;
};

