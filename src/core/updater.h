#ifndef UPDATER_H
#define UPDATER_H

#include <QObject>
#include <QString>
#include <QFile>

class QNetworkAccessManager;
class QNetworkReply;

class Updater : public QObject {
    Q_OBJECT
public:
    explicit Updater(QObject* parent = nullptr);

    void checkForUpdate();
    void downloadUpdate(const QString& url);
    void cleanup();

    QString zipPath() const { return m_zipPath; }

    static constexpr const char* GITHUB_OWNER = "pgschwend";
    static constexpr const char* GITHUB_REPO  = "rhenocalc";

signals:
    void updateAvailable(const QString& newVersion, const QString& downloadUrl);
    void noUpdateAvailable();
    void checkFailed(const QString& errorMsg);

    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void downloadFinished(const QString& zipPath);
    void updateError(const QString& errorMsg);

private slots:
    void onReleaseFetched(QNetworkReply* reply);
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void onDownloadFinished();

private:
    QNetworkAccessManager* m_nam;
    QNetworkReply* m_downloadReply = nullptr;
    QFile* m_downloadFile = nullptr;
    QString m_zipPath;
};

#endif // UPDATER_H

