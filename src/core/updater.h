#ifndef UPDATER_H
#define UPDATER_H

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

signals:
    void updateAvailable(const QString& newVersion, const QString& downloadUrl);
    void noUpdateAvailable();
    void checkFailed(const QString& errorMsg);

private slots:
    void onReleaseFetched(QNetworkReply* reply);

private:
    QNetworkAccessManager* m_nam;
};

#endif // UPDATER_H

