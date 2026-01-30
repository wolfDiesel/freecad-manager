#include "configsync.h"
#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QStandardPaths>
#include <QDebug>

ConfigSync::ConfigSync(QObject *parent)
    : IConfigSync(parent)
{
}

QString ConfigSync::getHomeConfigDir()
{
    return QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/FreeCAD";
}

QString ConfigSync::getHomeDataDir()
{
    return QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/FreeCAD";
}

bool ConfigSync::syncFromHome(const QString &portableConfigDir, const QString &portableDataDir)
{
    QString homeConfigDir = getHomeConfigDir();
    QString homeDataDir = getHomeDataDir();
    
    emit syncProgress(tr("Starting sync from home directory..."));
    
    if (!QDir(homeConfigDir).exists() && !QDir(homeDataDir).exists()) {
        emit syncError(tr("FreeCAD home configuration not found"));
        return false;
    }
    
    bool success = true;
    QStringList errors;
    
    if (QDir(homeConfigDir).exists()) {
        emit syncProgress(tr("Copying configuration from %1...").arg(homeConfigDir));
        if (!copyDirectory(homeConfigDir, portableConfigDir, true)) {
            success = false;
            errors << tr("Error copying configuration");
        } else {
            emit syncProgress(tr("Configuration copied"));
        }
    }
    
    if (QDir(homeDataDir).exists()) {
        emit syncProgress(tr("Copying data from %1...").arg(homeDataDir));
        if (!copyDirectory(homeDataDir, portableDataDir, true)) {
            success = false;
            errors << tr("Error copying data");
        } else {
            emit syncProgress(tr("Data copied"));
        }
    }
    
    if (success) {
        emit syncFinished(true, tr("Sync completed successfully"));
    } else {
        emit syncFinished(false, errors.join("\n"));
    }
    
    return success;
}

bool ConfigSync::syncToHome(const QString &portableConfigDir, const QString &portableDataDir)
{
    QString homeConfigDir = getHomeConfigDir();
    QString homeDataDir = getHomeDataDir();
    
    emit syncProgress(tr("Starting sync to home directory..."));
    
    if (!QDir(portableConfigDir).exists() && !QDir(portableDataDir).exists()) {
        emit syncError(tr("Portable configuration not found"));
        return false;
    }
    
    bool success = true;
    QStringList errors;
    
    if (QDir(portableConfigDir).exists()) {
        emit syncProgress(tr("Copying configuration to %1...").arg(homeConfigDir));
        if (!copyDirectory(portableConfigDir, homeConfigDir, true)) {
            success = false;
            errors << tr("Error copying configuration");
        } else {
            emit syncProgress(tr("Configuration copied"));
        }
    }
    
    if (QDir(portableDataDir).exists()) {
        emit syncProgress(tr("Copying data to %1...").arg(homeDataDir));
        if (!copyDirectory(portableDataDir, homeDataDir, true)) {
            success = false;
            errors << tr("Error copying data");
        } else {
            emit syncProgress(tr("Data copied"));
        }
    }
    
    if (success) {
        emit syncFinished(true, tr("Sync completed successfully"));
    } else {
        emit syncFinished(false, errors.join("\n"));
    }
    
    return success;
}

bool ConfigSync::copyDirectory(const QString &source, const QString &destination, bool overwrite)
{
    QDir sourceDir(source);
    if (!sourceDir.exists()) {
        return false;
    }
    
    QDir destDir;
    if (!destDir.mkpath(destination)) {
        emit syncError(tr("Failed to create directory: %1").arg(destination));
        return false;
    }
    
    QFileInfoList entries = sourceDir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QFileInfo &entry : entries) {
        QString sourcePath = entry.absoluteFilePath();
        QString destPath = destination + "/" + entry.fileName();
        if (entry.isDir()) {
            if (!copyDirectory(sourcePath, destPath, overwrite)) {
                return false;
            }
        } else {
            if (!copyFile(sourcePath, destPath, overwrite)) {
                return false;
            }
        }
    }
    
    return true;
}

bool ConfigSync::copyFile(const QString &source, const QString &destination, bool overwrite)
{
    QFileInfo destInfo(destination);
    if (destInfo.exists() && !overwrite) {
        return true;
    }
    QDir destDir = destInfo.absoluteDir();
    if (!destDir.exists()) {
        if (!destDir.mkpath(".")) {
            emit syncError(tr("Failed to create directory: %1").arg(destDir.absolutePath()));
            return false;
        }
    }
    if (destInfo.exists() && overwrite) {
        QFile::remove(destination);
    }
    if (!QFile::copy(source, destination)) {
        emit syncError(tr("Failed to copy file: %1 -> %2").arg(source, destination));
        return false;
    }
    
    return true;
}

