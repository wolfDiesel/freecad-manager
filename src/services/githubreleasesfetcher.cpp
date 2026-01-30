#include "githubreleasesfetcher.h"
#include <QNetworkRequest>
#include <QUrl>
#include <QUrlQuery>
#include "../constants.h"
#include "../utils/githubreleasesparser.h"

GitHubReleasesFetcher::GitHubReleasesFetcher(QObject *parent)
    : QObject(parent)
    , m_networkManager(nullptr)
    , m_releasesReply(nullptr)
{
    m_networkManager = new QNetworkAccessManager(this);
}

GitHubReleasesFetcher::~GitHubReleasesFetcher()
{
    cancel();
}

void GitHubReleasesFetcher::fetchReleases()
{
    if (m_releasesReply != nullptr) {
        return;
    }
    
    QUrl url(GitHub::ApiReleasesUrl);
    QUrlQuery query;
    query.addQueryItem("per_page", QString::number(GitHub::ReleasesPerPage));
    url.setQuery(query);
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", App::UserAgent);
    
    m_releasesReply = m_networkManager->get(request);
    connect(m_releasesReply, &QNetworkReply::finished, this, &GitHubReleasesFetcher::onReleasesDownloaded);
}

void GitHubReleasesFetcher::cancel()
{
    if (m_releasesReply) {
        m_releasesReply->abort();
        m_releasesReply->deleteLater();
        m_releasesReply = nullptr;
    }
}

void GitHubReleasesFetcher::onReleasesDownloaded()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    
    m_releasesReply = nullptr;
    
    if (reply->error() != QNetworkReply::NoError) {
        emit fetchError(reply->errorString());
        reply->deleteLater();
        return;
    }
    
    QByteArray data = reply->readAll();
    reply->deleteLater();
    
    QList<GitHubRelease> releases = GitHubReleasesParser::parse(data);
    
    if (releases.isEmpty() && !data.isEmpty()) {
        emit fetchError(tr("Failed to parse release data"));
    } else {
        emit releasesFetched(releases);
    }
}
