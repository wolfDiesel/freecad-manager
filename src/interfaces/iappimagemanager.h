#ifndef IAPPIMAGEMANAGER_H
#define IAPPIMAGEMANAGER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QList>
#include <QProcess>
#include "../types.h"

class IAppImageManager : public QObject
{
    Q_OBJECT
public:
    explicit IAppImageManager(QObject *parent = nullptr) : QObject(parent) {}

    virtual void setWorkingDirectory(const QString &dir) = 0;
    virtual QString workingDirectory() const = 0;
    virtual QList<AppImageInfo> scanAppImages() = 0;
    virtual void launchFreeCAD(const AppImageInfo &appImage, const QStringList &additionalParams = QStringList()) = 0;
    virtual void launchFreeCAD(const QString &appImagePath, const QStringList &additionalParams = QStringList()) = 0;
    virtual void stopFreeCAD() = 0;
    virtual bool isRunning() const = 0;
    virtual QString configPreset() const = 0;
    virtual void setConfigPreset(const QString &preset) = 0;

signals:
    void appImagesScanned(const QList<AppImageInfo> &images);
    void processFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void processError(QProcess::ProcessError error);
    void processOutput(const QString &output);
};

#endif
