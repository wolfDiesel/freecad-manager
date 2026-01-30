#include "appimagemanager.h"
#include <QDir>
#include <QFileInfo>
#include <QProcessEnvironment>
#include <QRegularExpression>
#include <QCoreApplication>
#include "../constants.h"
#include "../utils/versionextractor.h"

AppImageManager::AppImageManager(QObject *parent)
    : IAppImageManager(parent)
    , m_freecadProcess(nullptr)
    , m_configPreset("portable")
    , m_processDetached(false)
{
}

AppImageManager::~AppImageManager()
{
    if (m_freecadProcess) {
        m_freecadProcess->setParent(nullptr);
        m_freecadProcess = nullptr;
    }
}

void AppImageManager::setWorkingDirectory(const QString &dir)
{
    m_scriptDir = dir;
    m_fcadDir = PortableConfig::fcadDir(m_scriptDir);
}

QString AppImageManager::workingDirectory() const
{
    return m_scriptDir;
}

QString AppImageManager::configPreset() const
{
    return m_configPreset;
}

void AppImageManager::setConfigPreset(const QString &preset)
{
    m_configPreset = preset;
}

QList<AppImageInfo> AppImageManager::scanAppImages()
{
    QList<AppImageInfo> appImages;
    
    if (m_scriptDir.isEmpty()) {
        return appImages;
    }
    
    QDir dir(m_scriptDir);
    QStringList filters;
    filters << "FreeCAD*.appimage" << "freecad*.appimage";
    
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files | QDir::Executable);
    
    for (const QFileInfo &fileInfo : files) {
        AppImageInfo info;
        info.filePath = fileInfo.absoluteFilePath();
        info.fileName = fileInfo.fileName();
        info.fileSize = fileInfo.size();
        info.lastModified = fileInfo.lastModified();
        
        info.version = VersionExtractor::extractVersionFromFileName(info.fileName);
        
        appImages.append(info);
    }
    
    emit appImagesScanned(appImages);
    return appImages;
}

QString AppImageManager::extractVersionFromFileName(const QString &fileName) const
{
    return VersionExtractor::extractVersionFromFileName(fileName);
}

QString AppImageManager::extractVersionFromAppImage(const QString &filePath) const
{
    QProcess process;
    process.start(filePath, QStringList() << "--appimage-version");
    
    if (process.waitForFinished(3000)) {
        QString output = process.readAllStandardOutput().trimmed();
        if (!output.isEmpty()) {
            return output;
        }
    }
    
    return QString();
}

void AppImageManager::launchFreeCAD(const AppImageInfo &appImage, const QStringList &additionalParams)
{
    launchFreeCAD(appImage.filePath, additionalParams);
}

void AppImageManager::launchFreeCAD(const QString &appImagePath, const QStringList &additionalParams)
{
    if (appImagePath.isEmpty()) {
        emit processError(QProcess::FailedToStart);
        return;
    }
    
    if (m_processDetached && m_freecadProcess && m_freecadProcess->state() != QProcess::NotRunning) {
        return;
    }
    
    if (!m_freecadProcess || m_processDetached) {
        if (m_freecadProcess && m_processDetached) {
            m_freecadProcess->setParent(nullptr);
        }
        m_freecadProcess = new QProcess(this);
        m_processDetached = false;
        connect(m_freecadProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                this, &AppImageManager::onProcessFinished);
        connect(m_freecadProcess, &QProcess::errorOccurred, this, &AppImageManager::onProcessError);
        connect(m_freecadProcess, &QProcess::readyReadStandardOutput, [this]() {
            QString output = m_freecadProcess->readAllStandardOutput();
            emit processOutput(output);
        });
    }
    
    QFileInfo appImageInfo(appImagePath);
    QString baseDir = appImageInfo.absolutePath();
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    if (m_configPreset == "portable") {
        QString fcad = PortableConfig::fcadDir(baseDir);
        QString xdgConfig = PortableConfig::xdgConfigHome(baseDir);
        QString xdgData = PortableConfig::xdgDataHome(baseDir);
        QDir().mkpath(xdgConfig);
        QDir().mkpath(xdgData);
        env.insert("HOME", fcad);
        env.insert("XDG_CONFIG_HOME", xdgConfig);
        env.insert("XDG_DATA_HOME", xdgData);
    }
    m_freecadProcess->setProcessEnvironment(env);
    m_freecadProcess->start(appImagePath, additionalParams);
    
    if (!m_freecadProcess->waitForStarted(5000)) {
        emit processError(m_freecadProcess->error());
        return;
    }
    m_freecadProcess->setParent(nullptr);
    m_processDetached = true;
}

void AppImageManager::stopFreeCAD()
{
    if (m_freecadProcess && m_freecadProcess->state() != QProcess::NotRunning) {
        m_freecadProcess->kill();
        m_freecadProcess->waitForFinished();
    }
}

bool AppImageManager::isRunning() const
{
    if (m_processDetached) {
        return false;
    }
    return m_freecadProcess && m_freecadProcess->state() != QProcess::NotRunning;
}

void AppImageManager::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    emit processFinished(exitCode, exitStatus);
}

void AppImageManager::onProcessError(QProcess::ProcessError error)
{
    emit processError(error);
}
