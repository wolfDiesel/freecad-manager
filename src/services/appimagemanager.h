#ifndef APPIMAGEMANAGER_H
#define APPIMAGEMANAGER_H

#include <QProcess>
#include "../interfaces/iappimagemanager.h"

class AppImageManager : public IAppImageManager
{
    Q_OBJECT
public:
    explicit AppImageManager(QObject *parent = nullptr);
    ~AppImageManager();

    void setWorkingDirectory(const QString &dir) override;
    QString workingDirectory() const override;
    QList<AppImageInfo> scanAppImages() override;
    void launchFreeCAD(const AppImageInfo &appImage, const QStringList &additionalParams = QStringList()) override;
    void launchFreeCAD(const QString &appImagePath, const QStringList &additionalParams = QStringList()) override;
    void stopFreeCAD() override;
    bool isRunning() const override;
    QString configPreset() const override;
    void setConfigPreset(const QString &preset) override;

    QString extractVersionFromFileName(const QString &fileName) const;
    QString extractVersionFromAppImage(const QString &filePath) const;

private slots:
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessError(QProcess::ProcessError error);

private:
    void setupEnvironment(QProcess *process, const QString &baseDir);

    QString m_scriptDir;
    QString m_fcadDir;
    QProcess *m_freecadProcess;
    QString m_configPreset; // "portable" or "system"
    bool m_processDetached;
};

#endif
