#include "filedownloader.h"
#include <QNetworkRequest>
#include <QUrl>
#include <QFile>
#include "../constants.h"

FileDownloader::FileDownloader(QObject *parent)
    : QObject(parent)
    , m_networkManager(nullptr)
    , m_downloadReply(nullptr)
{
    m_networkManager = new QNetworkAccessManager(this);
}

FileDownloader::~FileDownloader()
{
    cancel();
}

void FileDownloader::downloadFile(const QString &url, const QString &savePath)
{
    if (m_downloadReply != nullptr) {
        emit downloadError(tr("Download already in progress"));
        return;
    }
    
    m_downloadSavePath = savePath;
    
    QUrl downloadUrl(url);
    QNetworkRequest request(downloadUrl);
    request.setRawHeader("User-Agent", App::UserAgent);
    
    m_downloadReply = m_networkManager->get(request);
    connect(m_downloadReply, &QNetworkReply::downloadProgress, this, &FileDownloader::onDownloadProgress);
    connect(m_downloadReply, &QNetworkReply::finished, this, &FileDownloader::onDownloadFinished);
}

void FileDownloader::cancel()
{
    if (m_downloadReply) {
        m_downloadReply->abort();
        m_downloadReply->deleteLater();
        m_downloadReply = nullptr;
    }
}

void FileDownloader::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    emit downloadProgress(bytesReceived, bytesTotal);
}

void FileDownloader::onDownloadFinished()
{
    if (!m_downloadReply) return;
    
    bool success = false;
    QString errorMsg;
    
    if (m_downloadReply->error() != QNetworkReply::NoError) {
        errorMsg = m_downloadReply->errorString();
        emit downloadError(errorMsg);
    } else {
        QFile file(m_downloadSavePath);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(m_downloadReply->readAll());
            file.close();
            QFile::setPermissions(m_downloadSavePath, QFile::ReadUser | QFile::WriteUser | QFile::ExeUser |
                                                 QFile::ReadGroup | QFile::ExeGroup |
                                                 QFile::ReadOther | QFile::ExeOther);
            
            success = true;
            emit downloadFinished(m_downloadSavePath, true);
        } else {
            errorMsg = file.errorString();
            emit downloadError(tr("Save error: %1").arg(errorMsg));
        }
    }
    
    m_downloadReply->deleteLater();
    m_downloadReply = nullptr;
    
    if (!success) {
        emit downloadFinished(m_downloadSavePath, false);
    }
}
