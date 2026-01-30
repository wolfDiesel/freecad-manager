#include "downloader.h"
#include "githubreleasesfetcher.h"
#include "filedownloader.h"
#include "../helpers/filesystemchecker.h"

Downloader::Downloader(QObject *parent)
    : IDownloader(parent)
    , m_releasesFetcher(nullptr)
    , m_fileDownloader(nullptr)
{
    m_releasesFetcher = new GitHubReleasesFetcher(this);
    m_fileDownloader = new FileDownloader(this);
    connect(m_releasesFetcher, &GitHubReleasesFetcher::releasesFetched, 
            this, &Downloader::onReleasesFetched);
    connect(m_releasesFetcher, &GitHubReleasesFetcher::fetchError, 
            this, &Downloader::onFetchError);
    
    connect(m_fileDownloader, &FileDownloader::downloadProgress, 
            this, &Downloader::onDownloadProgress);
    connect(m_fileDownloader, &FileDownloader::downloadFinished, 
            this, &Downloader::onDownloadFinished);
    connect(m_fileDownloader, &FileDownloader::downloadError, 
            this, &Downloader::onDownloadError);
}

Downloader::~Downloader()
{
    cancelDownload();
}

void Downloader::fetchReleases()
{
    m_releasesFetcher->fetchReleases();
}

void Downloader::downloadFile(const QString &url, const QString &savePath)
{
    m_fileDownloader->downloadFile(url, savePath);
}

void Downloader::cancelDownload()
{
    m_releasesFetcher->cancel();
    m_fileDownloader->cancel();
}

bool Downloader::isDownloading() const
{
    return m_fileDownloader->isDownloading();
}

QList<GitHubRelease> Downloader::releases() const
{
    return m_githubReleases;
}

void Downloader::setWorkingDirectory(const QString &dir)
{
    m_workingDir = dir;
}

QString Downloader::workingDirectory() const
{
    return m_workingDir;
}

ReleaseDownloadStatus Downloader::releaseDownloadStatus(const GitHubRelease &release) const
{
    ReleaseDownloadStatus status;
    for (const AppImageAsset &img : release.appImages) {
        if (FileSystemChecker::isFileDownloaded(img.fileName, m_workingDir)) {
            status.isDownloaded = true;
            status.architecture = img.architecture;
            break;
        }
    }
    return status;
}

bool Downloader::isFileDownloaded(const QString &fileName) const
{
    return FileSystemChecker::isFileDownloaded(fileName, m_workingDir);
}

void Downloader::onReleasesFetched(const QList<GitHubRelease> &releases)
{
    m_githubReleases = releases;
    emit releasesFetched(releases);
}

void Downloader::onFetchError(const QString &error)
{
    emit fetchError(error);
}

void Downloader::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    emit downloadProgress(bytesReceived, bytesTotal);
}

void Downloader::onDownloadFinished(const QString &filePath, bool success)
{
    emit downloadFinished(filePath, success);
}

void Downloader::onDownloadError(const QString &error)
{
    emit downloadError(error);
}
