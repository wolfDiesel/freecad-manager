#ifndef FILEDOWNLOADER_H
#define FILEDOWNLOADER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QString>

class FileDownloader : public QObject
{
    Q_OBJECT

public:
    explicit FileDownloader(QObject *parent = nullptr);
    ~FileDownloader();

    void downloadFile(const QString &url, const QString &savePath);
    void cancel();
    bool isDownloading() const { return m_downloadReply != nullptr; }

signals:
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void downloadFinished(const QString &filePath, bool success);
    void downloadError(const QString &error);

private slots:
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void onDownloadFinished();

private:
    QNetworkAccessManager *m_networkManager;
    QNetworkReply *m_downloadReply;
    QString m_downloadSavePath;
};

#endif
