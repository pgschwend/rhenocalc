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
#include <QCoreApplication>
#include <QDir>

Updater::Updater(QObject* parent)
    : QObject(parent)
    , m_nam(new QNetworkAccessManager(this))
{
}

void Updater::checkForUpdate()
{
    QString url = QString("https://api.github.com/repos/%1/%2/releases/latest")
                      .arg(GITHUB_OWNER, GITHUB_REPO);
    QNetworkRequest req{QUrl(url)};
    req.setHeader(QNetworkRequest::UserAgentHeader, "RhenoCalc-Updater");

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

void Updater::downloadUpdate(const QString& url)
{
    // Store update ZIP in app directory
    QString appDir = QCoreApplication::applicationDirPath();
    m_zipPath = appDir + "/rhenocalc_update.zip";

    // Clean up old file
    QFile::remove(m_zipPath);

    m_downloadFile = new QFile(m_zipPath);  // No parent - we manage lifetime manually
    if (!m_downloadFile->open(QIODevice::WriteOnly)) {
        delete m_downloadFile;
        m_downloadFile = nullptr;
        emit updateError("Could not create temporary file for download.");
        return;
    }

    QNetworkRequest req{QUrl(url)};
    req.setHeader(QNetworkRequest::UserAgentHeader, "RhenoCalc-Updater");
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);

    m_downloadReply = m_nam->get(req);
    connect(m_downloadReply, &QNetworkReply::downloadProgress, this, &Updater::onDownloadProgress);
    connect(m_downloadReply, &QNetworkReply::readyRead, this, [this]() {
        if (m_downloadFile && m_downloadReply) {
            m_downloadFile->write(m_downloadReply->readAll());
        }
    });
    connect(m_downloadReply, &QNetworkReply::finished, this, &Updater::onDownloadFinished);
}

void Updater::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    emit downloadProgress(bytesReceived, bytesTotal);
}

void Updater::onDownloadFinished()
{
    if (!m_downloadReply || !m_downloadFile) {
        emit updateError("Download state invalid.");
        return;
    }

    // Write any remaining data and close file immediately
    m_downloadFile->write(m_downloadReply->readAll());
    m_downloadFile->flush();
    m_downloadFile->close();
    delete m_downloadFile;
    m_downloadFile = nullptr;

    if (m_downloadReply->error() != QNetworkReply::NoError) {
        QString error = m_downloadReply->errorString();
        m_downloadReply->close();
        delete m_downloadReply;
        m_downloadReply = nullptr;
        emit updateError(QString("Download failed: %1").arg(error));
        return;
    }

    m_downloadReply->close();
    delete m_downloadReply;
    m_downloadReply = nullptr;

    emit downloadFinished(m_zipPath);
}

void Updater::cleanup()
{
    // Abort any ongoing download - must delete synchronously since quit() follows
    if (m_downloadReply) {
        m_downloadReply->abort();
        m_downloadReply->close();
        delete m_downloadReply;
        m_downloadReply = nullptr;
    }

    // Close and delete download file handle - MUST be synchronous
    if (m_downloadFile) {
        if (m_downloadFile->isOpen()) {
            m_downloadFile->close();
        }
        delete m_downloadFile;
        m_downloadFile = nullptr;
    }

    // Clear all network connections
    m_nam->clearConnectionCache();
    m_nam->clearAccessCache();
}

