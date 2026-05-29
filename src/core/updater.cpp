#include "updater.h"
#include "info.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QVersionNumber>
#include <QRegularExpression>

Updater::Updater(QObject* parent)
    : QObject(parent)
    , m_nam(new QNetworkAccessManager(this))
{
    connect(m_nam, &QNetworkAccessManager::finished, this, &Updater::onReleaseFetched);
}

void Updater::checkForUpdate()
{
    QString url = QString("https://api.github.com/repos/%1/%2/releases/latest")
                      .arg(GITHUB_OWNER, GITHUB_REPO);
    QNetworkRequest req{QUrl(url)};
    req.setHeader(QNetworkRequest::UserAgentHeader, "RhenoCalc-Updater");
    m_nam->get(req);
}

void Updater::onReleaseFetched(QNetworkReply* reply)
{
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        emit checkFailed(reply->errorString());
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    QJsonObject obj = doc.object();

    QString tagName = obj["tag_name"].toString(); // e.g. "V0.2.0"
    QString remoteVersion = tagName;
    remoteVersion.remove(QRegularExpression("^[vV]"));

    QString localVersion = QString(APP_VERSION_STRING);
    localVersion.remove(QRegularExpression("^[vV]"));

    QVersionNumber remote = QVersionNumber::fromString(remoteVersion);
    QVersionNumber local  = QVersionNumber::fromString(localVersion);

    if (remote > local) {
        // Find a .zip asset containing "win" in its name
        QString downloadUrl;
        QJsonArray assets = obj["assets"].toArray();
        for (const auto& asset : assets) {
            QJsonObject a = asset.toObject();
            QString name = a["name"].toString();
            if (name.endsWith(".zip") && name.contains("win", Qt::CaseInsensitive)) {
                downloadUrl = a["browser_download_url"].toString();
                break;
            }
        }
        // Fallback: first .zip asset
        if (downloadUrl.isEmpty()) {
            for (const auto& asset : assets) {
                QJsonObject a = asset.toObject();
                if (a["name"].toString().endsWith(".zip")) {
                    downloadUrl = a["browser_download_url"].toString();
                    break;
                }
            }
        }
        // Last fallback: zipball
        if (downloadUrl.isEmpty()) {
            downloadUrl = obj["zipball_url"].toString();
        }
        if (!downloadUrl.isEmpty()) {
            emit updateAvailable(tagName, downloadUrl);
        } else {
            emit checkFailed("Update found but no download asset available.");
        }
    } else {
        emit noUpdateAvailable();
    }
}

