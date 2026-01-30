#include "filesystemchecker.h"
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>
#include "../utils/versionextractor.h"

bool FileSystemChecker::isFileDownloaded(const QString &fileName, const QString &workingDir)
{
    if (workingDir.isEmpty()) {
        return false;
    }
    QString exactPath = workingDir + "/" + fileName;
    if (QFile::exists(exactPath)) {
        return true;
    }
    QString lowerPath = workingDir + "/" + fileName.toLower();
    if (QFile::exists(lowerPath)) {
        return true;
    }
    QString baseName = fileName;
    QString extension;
    if (baseName.endsWith(".AppImage", Qt::CaseInsensitive)) {
        extension = baseName.right(9);
        baseName = baseName.left(baseName.length() - 9);
    }
    if (!extension.isEmpty()) {
        QString altExtension = (extension == ".AppImage") ? ".appimage" : ".AppImage";
        QString altPath = workingDir + "/" + baseName + altExtension;
        if (QFile::exists(altPath)) {
            return true;
        }
    }
    QDir dir(workingDir);
    QStringList filters;
    filters << baseName + "*.appimage" << baseName + "*.AppImage";
    
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files);
    if (!files.isEmpty()) {
        return true;
    }
    if (checkByVersion(fileName, workingDir)) {
        return true;
    }
    if (baseName.contains("weekly-")) {
        QRegularExpression weeklyRx("weekly-\\d{4}\\.\\d{2}\\.\\d{2}");
        QRegularExpressionMatch match = weeklyRx.match(baseName);
        if (match.hasMatch()) {
            QString weeklyPart = match.captured(0);
            filters.clear();
            filters << "*" + weeklyPart + "*.appimage" << "*" + weeklyPart + "*.AppImage";
            files = dir.entryInfoList(filters, QDir::Files);
            if (!files.isEmpty()) {
                return true;
            }
        }
    }
    
    return false;
}

bool FileSystemChecker::checkByVersion(const QString &fileName, const QString &workingDir)
{
    QString version = VersionExtractor::extractVersionFromFileName(fileName);
    if (version.isEmpty()) {
        return false;
    }
    QDir dir(workingDir);
    QStringList filters;
    filters << "FreeCAD*.appimage" << "freecad*.appimage";
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files);
    
    for (const QFileInfo &fileInfo : files) {
        QString fileVersion = VersionExtractor::extractVersionFromFileName(fileInfo.fileName());
        if (fileVersion == version) {
            return true;
        }
    }
    
    return false;
}
