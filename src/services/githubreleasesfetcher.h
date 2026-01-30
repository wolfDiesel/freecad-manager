#ifndef GITHUBRELEASESFETCHER_H
#define GITHUBRELEASESFETCHER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QList>
#include "../types.h"

class GitHubReleasesFetcher : public QObject
{
    Q_OBJECT

public:
    explicit GitHubReleasesFetcher(QObject *parent = nullptr);
    ~GitHubReleasesFetcher();

    void fetchReleases();
    void cancel();

signals:
    void releasesFetched(const QList<GitHubRelease> &releases);
    void fetchError(const QString &error);

private slots:
    void onReleasesDownloaded();

private:
    QNetworkAccessManager *m_networkManager;
    QNetworkReply *m_releasesReply;
};

#endif
