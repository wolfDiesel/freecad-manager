#ifndef GITHUBRELEASESPARSER_H
#define GITHUBRELEASESPARSER_H

#include <QByteArray>
#include <QList>
#include "../types.h"

enum class ReleaseType {
    Weekly,
    RC,
    Stable,
    Unknown
};

class GitHubReleasesParser
{
public:
    static QList<GitHubRelease> parse(const QByteArray &jsonData);
    static QList<AppImageAsset> extractAppImages(const QJsonArray &assets);
    static GitHubRelease parseRelease(const QJsonObject &releaseObj);
    static ReleaseType getReleaseType(const GitHubRelease &release);
    static QList<GitHubRelease> filterReleases(const QList<GitHubRelease> &releases);
};

#endif
