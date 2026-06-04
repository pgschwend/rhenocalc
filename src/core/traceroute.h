#pragma once

#include <QObject>
#include <QThread>
#include <QHostAddress>

struct TraceHop {
    int hop = 0;
    QString address;
    int rtt = -1;  // Round-trip time in ms, -1 = timeout
    bool isDestination = false;
};

class TracerouteWorker : public QObject {
    Q_OBJECT

public:
    explicit TracerouteWorker(const QString& host, int maxHops = 30, int timeout = 3000);
    ~TracerouteWorker() override;

public slots:
    void start();
    void stop();

signals:
    void hopResult(const TraceHop& hop);
    void finished(bool success, const QString& message);
    void error(const QString& message);

private:
    bool resolveHost();
    void traceHop(int ttl);

    QString m_host;
    QHostAddress m_targetAddress;
    int m_maxHops;
    int m_timeout;
    bool m_running = false;

#ifdef Q_OS_WIN
    void* m_icmpHandle = nullptr;
#endif
};

class Traceroute : public QObject {
    Q_OBJECT

public:
    explicit Traceroute(QObject* parent = nullptr);
    ~Traceroute() override;

    void start(const QString& host, int maxHops = 30, int timeout = 3000);
    void stop();
    [[nodiscard]] bool isRunning() const { return m_running; }

signals:
    void hopResult(const TraceHop& hop);
    void finished(bool success, const QString& message);
    void error(const QString& message);

private:
    QThread* m_thread = nullptr;
    TracerouteWorker* m_worker = nullptr;
    bool m_running = false;
};

