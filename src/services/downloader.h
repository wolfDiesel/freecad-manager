#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <QString>
#include <QList>
#include "../interfaces/idownloader.h"
#include "../types.h"

class GitHubReleasesFetcher;
class FileDownloader;

class Downloader : public IDownloader
{
    Q_OBJECT
public:
    explicit Downloader(QObject *parent = nullptr);
    ~Downloader();

    void fetchReleases() override;
    void downloadFile(const QString &url, const QString &savePath) override;
    void cancelDownload() override;
    bool isDownloading() const override;
    void setWorkingDirectory(const QString &dir) override;
    QString workingDirectory() const override;
    ReleaseDownloadStatus releaseDownloadStatus(const GitHubRelease &release) const override;
    bool isFileDownloaded(const QString &fileName) const override;

    QList<GitHubRelease> releases() const;

private slots:
    void onReleasesFetched(const QList<GitHubRelease> &releases);
    void onFetchError(const QString &error);
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void onDownloadFinished(const QString &filePath, bool success);
    void onDownloadError(const QString &error);

private:
    GitHubReleasesFetcher *m_releasesFetcher;
    FileDownloader *m_fileDownloader;
    QList<GitHubRelease> m_githubReleases;
    QString m_workingDir;
};

#endif
