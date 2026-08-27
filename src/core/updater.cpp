#include "updater.h"
#include "info.h"

#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QVersionNumber>
#include <QRegularExpression>

namespace Rheno::Core {

Updater::Updater(QObject* parent)
    : QObject(parent)
    , m_nam(new QNetworkAccessManager(this))
{
}

QString Updater::releasesPageUrl()
{
    return QString("https://github.com/%1/%2/releases").arg(GITHUB_OWNER, GITHUB_REPO);
}

void Updater::checkForUpdate()
{
    QString url = QString("https://api.github.com/repos/%1/%2/releases/latest")
                      .arg(GITHUB_OWNER, GITHUB_REPO);
    QNetworkRequest req{QUrl(url)};
    req.setHeader(QNetworkRequest::UserAgentHeader, "RhenoCalc-Updater");
    req.setTransferTimeout(10000);

    QNetworkReply* reply = m_nam->get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onReleaseFetched(reply);
    });
}

void Updater::onReleaseFetched(QNetworkReply* reply)
{
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        emit checkFailed(reply->errorString());
        return;
    }

    const QByteArray body = reply->readAll();
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(body, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        emit checkFailed("Malformed response from update server");
        return;
    }
    QJsonObject obj = doc.object();

    if (!obj.contains("tag_name") || !obj["tag_name"].isString()) {
        emit checkFailed("Unexpected response from update server");
        return;
    }

    QString tagName = obj["tag_name"].toString();
    QString remoteVersion = tagName;
    remoteVersion.remove(QRegularExpression("^[vV]"));

    QString localVersion = QString(APP_VERSION_STRING);
    localVersion.remove(QRegularExpression("^[vV]"));

    QVersionNumber remote = QVersionNumber::fromString(remoteVersion);
    QVersionNumber local  = QVersionNumber::fromString(localVersion);

    if (remote > local) {
        // Return the HTML URL to the release page
        QString releaseUrl = obj["html_url"].toString();
        if (releaseUrl.isEmpty()) {
            releaseUrl = releasesPageUrl();
        }
        emit updateAvailable(tagName, releaseUrl);
    } else {
        emit noUpdateAvailable();
    }
}

} // namespace Rheno::Core

