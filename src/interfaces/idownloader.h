#ifndef IDOWNLOADER_H
#define IDOWNLOADER_H

#include <QObject>
#include <QString>
#include <QList>
#include "../types.h"

struct ReleaseDownloadStatus {
    bool isDownloaded = false;
    QString architecture;
};

class IDownloader : public QObject
{
    Q_OBJECT
public:
    explicit IDownloader(QObject *parent = nullptr) : QObject(parent) {}

    virtual void fetchReleases() = 0;
    virtual void downloadFile(const QString &url, const QString &savePath) = 0;
    virtual void cancelDownload() = 0;
    virtual bool isDownloading() const = 0;
    virtual void setWorkingDirectory(const QString &dir) = 0;
    virtual QString workingDirectory() const = 0;
    virtual ReleaseDownloadStatus releaseDownloadStatus(const GitHubRelease &release) const = 0;
    virtual bool isFileDownloaded(const QString &fileName) const = 0;

signals:
    void releasesFetched(const QList<GitHubRelease> &releases);
    void fetchError(const QString &error);
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void downloadFinished(const QString &filePath, bool success);
    void downloadError(const QString &error);
};

#endif
