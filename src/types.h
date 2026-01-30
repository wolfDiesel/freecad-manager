#ifndef TYPES_H
#define TYPES_H

#include <QString>
#include <QDateTime>
#include <QList>
#include <QMetaType>

struct AppImageInfo {
    QString filePath;
    QString fileName;
    QString version;
    qint64 fileSize;
    QDateTime lastModified;
};

Q_DECLARE_METATYPE(AppImageInfo)

struct AppImageAsset {
    QString fileName;
    QString downloadUrl;
    qint64 fileSize;
    QString architecture;
};

struct GitHubRelease {
    QString tagName;
    QString name;
    QString publishedAt;
    QList<AppImageAsset> appImages;
};

#endif
