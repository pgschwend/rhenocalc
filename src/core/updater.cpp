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
#include <QStandardPaths>
#include <QCoreApplication>
#include <QDir>
#include <QProcess>
#include <QDebug>

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
    // Store update files in app directory, not temp
    QString appDir = QCoreApplication::applicationDirPath();
    m_zipPath = appDir + "/rhenocalc_update.zip";
    m_extractedDir = appDir + "/rhenocalc_update_temp";

    // Clean up old files
    QFile::remove(m_zipPath);
    QDir(m_extractedDir).removeRecursively();

    m_downloadFile = new QFile(m_zipPath, this);
    if (!m_downloadFile->open(QIODevice::WriteOnly)) {
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

    // Write any remaining data
    m_downloadFile->write(m_downloadReply->readAll());
    m_downloadFile->close();

    if (m_downloadReply->error() != QNetworkReply::NoError) {
        emit updateError(QString("Download failed: %1").arg(m_downloadReply->errorString()));
        m_downloadReply->deleteLater();
        m_downloadReply = nullptr;
        return;
    }

    m_downloadReply->deleteLater();
    m_downloadReply = nullptr;

    emit downloadFinished(m_zipPath);
}

void Updater::extractUpdate(const QString& zipPath)
{
    // Use PowerShell to extract
    QProcess* proc = new QProcess(this);
    connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this, proc](int exitCode, QProcess::ExitStatus) {
        proc->deleteLater();

        if (exitCode != 0) {
            emit updateError("Extraction failed.");
            return;
        }

        // GitHub release ZIPs often have a single root folder - detect that
        QDir extractedDir(m_extractedDir);
        QStringList entries = extractedDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

        QString finalDir = m_extractedDir;
        if (entries.size() == 1) {
            // Single subfolder - use that as the source
            finalDir = m_extractedDir + "/" + entries.first();
        }

        m_extractedDir = finalDir;
        emit extractFinished(finalDir);
    });

    QString cmd = QString("Expand-Archive -Path \"%1\" -DestinationPath \"%2\" -Force")
                      .arg(zipPath, m_extractedDir);
    proc->start("powershell", QStringList{"-NoProfile", "-Command", cmd});
}

void Updater::cleanup()
{
    // Abort any ongoing download
    if (m_downloadReply) {
        m_downloadReply->abort();
        m_downloadReply->deleteLater();
        m_downloadReply = nullptr;
    }

    // Close and delete download file
    if (m_downloadFile) {
        if (m_downloadFile->isOpen()) {
            m_downloadFile->close();
        }
        m_downloadFile->deleteLater();
        m_downloadFile = nullptr;
    }

    // Clear network cache/connections
    m_nam->clearConnectionCache();

    // Remove downloaded ZIP (extractedDir is needed by batch script)
    if (!m_zipPath.isEmpty()) {
        QFile::remove(m_zipPath);
    }
}

