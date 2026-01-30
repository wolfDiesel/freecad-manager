#include "githubreleasesparser.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QDateTime>
#include <QRegularExpression>
#include <QDebug>
#include <algorithm>
#include "architecturedetector.h"

QList<GitHubRelease> GitHubReleasesParser::parse(const QByteArray &jsonData)
{
    QList<GitHubRelease> releases;
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &error);
    
    if (error.error != QJsonParseError::NoError) {
        return releases;
    }
    
    QJsonArray releasesArray = doc.array();
    int totalReleases = releasesArray.size();
    int releasesWithAppImage = 0;
    
    for (const QJsonValue &value : releasesArray) {
        QJsonObject releaseObj = value.toObject();
        GitHubRelease release = parseRelease(releaseObj);
        
        if (!release.appImages.isEmpty()) {
            releases.append(release);
            releasesWithAppImage++;
        }
    }
    
#if defined(QT_DEBUG)
    qDebug() << "GitHubReleasesParser: Всего релизов с GitHub:" << totalReleases;
    qDebug() << "GitHubReleasesParser: Релизов с AppImage:" << releasesWithAppImage;
#endif
    QList<GitHubRelease> filtered = filterReleases(releases);
#if defined(QT_DEBUG)
    qDebug() << "GitHubReleasesParser: После фильтрации:" << filtered.size();
#endif
    
    return filtered;
}

ReleaseType GitHubReleasesParser::getReleaseType(const GitHubRelease &release)
{
    QString tagName = release.tagName.toLower();
    QString name = release.name.toLower();
    QString fileName = release.appImages.isEmpty() ? QString() : release.appImages.first().fileName.toLower();
    
    bool isWeekly = false;
    QRegularExpression weeklyFormatRx("weekly[-_]\\d{4}[-_.]\\d{2}[-_.]\\d{2}");
    if (weeklyFormatRx.match(tagName).hasMatch() || weeklyFormatRx.match(name).hasMatch()) {
        isWeekly = true;
    }
    if (!isWeekly && (tagName.startsWith("weekly") || name.startsWith("weekly"))) {
        isWeekly = true;
    }
    QRegularExpression weeklyWordRx("\\bweekly\\b");
    if (!isWeekly && (weeklyWordRx.match(tagName).hasMatch() || weeklyWordRx.match(name).hasMatch())) {
        isWeekly = true;
    }
    
    if (isWeekly) {
        return ReleaseType::Weekly;
    }
    QRegularExpression rcVersionRx("\\d+\\.\\d+rc\\d+");
    if (rcVersionRx.match(tagName).hasMatch() || rcVersionRx.match(name).hasMatch()) {
        return ReleaseType::RC;
    }
    if ((tagName.contains("-rc") || tagName.contains("_rc") || 
         name.contains("-rc") || name.contains("_rc") ||
         fileName.contains("-rc") || fileName.contains("_rc")) &&
        !tagName.contains("weekly") && !name.contains("weekly") && !fileName.contains("weekly")) {
        return ReleaseType::RC;
    }
    if ((tagName.contains("release") && tagName.contains("candidate")) ||
        (name.contains("release") && name.contains("candidate"))) {
        return ReleaseType::RC;
    }
    QRegularExpression stableVersionRx("\\d+\\.\\d+\\.\\d+");
    QRegularExpressionMatch tagMatch = stableVersionRx.match(tagName);
    QRegularExpressionMatch nameMatch = stableVersionRx.match(name);
    
    if (tagMatch.hasMatch() || nameMatch.hasMatch()) {
        QString combined = tagName + " " + name + " " + fileName;
        bool hasRcInVersion = rcVersionRx.match(tagName).hasMatch() || rcVersionRx.match(name).hasMatch();
        if (!hasRcInVersion && !combined.contains("weekly")) {
            return ReleaseType::Stable;
        }
    }
    QRegularExpression majorMinorRx("^v?\\d+\\.\\d+$");
    if (majorMinorRx.match(tagName).hasMatch() || majorMinorRx.match(name).hasMatch()) {
        return ReleaseType::Stable;
    }
    QRegularExpression anyVersionRx("^v?\\d+\\.\\d+");
    if (anyVersionRx.match(tagName).hasMatch() || anyVersionRx.match(name).hasMatch()) {
        QString combined = tagName + " " + name + " " + fileName;
        bool hasRcInVersion = rcVersionRx.match(tagName).hasMatch() || rcVersionRx.match(name).hasMatch();
        if (!hasRcInVersion && !combined.contains("weekly")) {
            return ReleaseType::Stable;
        }
    }
    QRegularExpression anyVersionPattern("\\d+\\.\\d+");
    if (anyVersionPattern.match(tagName).hasMatch() || anyVersionPattern.match(name).hasMatch()) {
        QString combined = tagName + " " + name + " " + fileName;
        bool hasRcInVersion = rcVersionRx.match(tagName).hasMatch() || rcVersionRx.match(name).hasMatch();
        if (!hasRcInVersion && !combined.contains("weekly") && !combined.contains("rc")) {
            return ReleaseType::Stable;
        }
    }
    
    return ReleaseType::Unknown;
}

QList<GitHubRelease> GitHubReleasesParser::filterReleases(const QList<GitHubRelease> &releases)
{
    if (releases.isEmpty()) {
        return releases;
    }
    QList<GitHubRelease> sortedReleases = releases;
    std::sort(sortedReleases.begin(), sortedReleases.end(), 
        [](const GitHubRelease &a, const GitHubRelease &b) {
            QDateTime dateA = QDateTime::fromString(a.publishedAt, Qt::ISODate);
            QDateTime dateB = QDateTime::fromString(b.publishedAt, Qt::ISODate);
            return dateA > dateB;
        });
    QList<GitHubRelease> weeklyReleases;
    QList<GitHubRelease> rcReleases;
    QList<GitHubRelease> otherReleases;
    int weeklyCount = 0, rcCount = 0, stableCount = 0, unknownCount = 0;
    
    for (const GitHubRelease &release : sortedReleases) {
        ReleaseType type = getReleaseType(release);
        
        if (type == ReleaseType::Weekly) {
            weeklyReleases.append(release);
            weeklyCount++;
        } else if (type == ReleaseType::RC) {
            rcReleases.append(release);
            rcCount++;
        } else if (type == ReleaseType::Stable) {
            otherReleases.append(release);
            stableCount++;
        } else {
            otherReleases.append(release);
            unknownCount++;
        }
    }
    
#if defined(QT_DEBUG)
    qDebug() << "GitHubReleasesParser: Распределение по типам - Weekly:" << weeklyCount 
             << "RC:" << rcCount << "Stable:" << stableCount << "Unknown:" << unknownCount;
#endif
    if (weeklyReleases.size() > 5) {
        weeklyReleases = weeklyReleases.mid(0, 5);
    }
    if (rcReleases.size() > 3) {
        rcReleases = rcReleases.mid(0, 3);
    }
    int alreadySelected = weeklyReleases.size() + rcReleases.size();
    int limitForOthers = 30 - alreadySelected;
    
#if defined(QT_DEBUG)
    qDebug() << "GitHubReleasesParser: Уже выбрано (weekly+rc):" << alreadySelected 
             << "Лимит для остальных:" << limitForOthers 
             << "Доступно остальных:" << otherReleases.size();
#endif
    if (limitForOthers > 0) {
        if (otherReleases.size() > limitForOthers) {
            otherReleases = otherReleases.mid(0, limitForOthers);
        }
    } else {
        otherReleases.clear();
    }
    
#if defined(QT_DEBUG)
    qDebug() << "GitHubReleasesParser: Итоговое количество - Weekly:" << weeklyReleases.size()
             << "RC:" << rcReleases.size() << "Остальные:" << otherReleases.size();
#endif
    QList<GitHubRelease> result;
    result.append(weeklyReleases);
    result.append(rcReleases);
    result.append(otherReleases);
    std::sort(result.begin(), result.end(), 
        [](const GitHubRelease &a, const GitHubRelease &b) {
            QDateTime dateA = QDateTime::fromString(a.publishedAt, Qt::ISODate);
            QDateTime dateB = QDateTime::fromString(b.publishedAt, Qt::ISODate);
            return dateA > dateB;
        });
    
    return result;
}

QList<AppImageAsset> GitHubReleasesParser::extractAppImages(const QJsonArray &assets)
{
    QList<AppImageAsset> appImageList;
    
    for (const QJsonValue &assetValue : assets) {
        QJsonObject asset = assetValue.toObject();
        QString assetName = asset["name"].toString();
        
        if (assetName.endsWith(".AppImage", Qt::CaseInsensitive)) {
            AppImageAsset appImage;
            appImage.fileName = assetName;
            appImage.downloadUrl = asset["browser_download_url"].toString();
            appImage.fileSize = asset["size"].toVariant().toLongLong();
            appImage.architecture = ArchitectureDetector::detectArchitecture(assetName);
            
            appImageList.append(appImage);
        }
    }
    
    return appImageList;
}

GitHubRelease GitHubReleasesParser::parseRelease(const QJsonObject &releaseObj)
{
    GitHubRelease releaseInfo;
    
    releaseInfo.tagName = releaseObj["tag_name"].toString();
    releaseInfo.name = releaseObj["name"].toString();
    if (releaseInfo.name.isEmpty()) {
        releaseInfo.name = releaseInfo.tagName;
    }
    releaseInfo.publishedAt = releaseObj["published_at"].toString();
    
    QJsonArray assets = releaseObj["assets"].toArray();
    releaseInfo.appImages = extractAppImages(assets);
    return releaseInfo;
}
