#pragma once

#include <QObject>
#include <QString>

class QNetworkAccessManager;
class QNetworkReply;

class Updater : public QObject {
    Q_OBJECT
public:
    explicit Updater(QObject* parent = nullptr);

    void checkForUpdate();

    static constexpr const char* GITHUB_OWNER = "pgschwend";
    static constexpr const char* GITHUB_REPO  = "rhenocalc";

    // Returns the GitHub releases page URL
    static QString releasesPageUrl();

signals:
    void updateAvailable(const QString& newVersion, const QString& releaseUrl);
    void noUpdateAvailable();
    void checkFailed(const QString& errorMsg);

private slots:
    void onReleaseFetched(QNetworkReply* reply);

private:
    QNetworkAccessManager* m_nam = nullptr;
};
