#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QTabWidget>
#include <QTableWidget>
#include <QProgressBar>
#include <QMessageBox>
#include <QGroupBox>
#include <QScrollBar>
#include <QFileDialog>
#include <QCoreApplication>
#include <QDateTime>
#include <QBrush>
#include <QCloseEvent>
#include <QTranslator>
#include <QEvent>
#include "types.h"
#include "interfaces/idownloader.h"
#include "interfaces/iappimagemanager.h"
#include "interfaces/iconfigsync.h"
#include "interfaces/itraymanager.h"

class ReleasesTableController;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(IDownloader *downloader, IAppImageManager *appImageManager,
                        IConfigSync *configSync, ITrayManager *trayManager,
                        QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onScanAppImages();
    void onAppImageSelected();
    void onLaunchClicked();
    void onLaunchWithParamsClicked();
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessError(QProcess::ProcessError error);
    void onFetchGitHubReleases();
    void onReleasesFetched(const QList<GitHubRelease> &releases);
    void onFetchError(const QString &error);
    void onDownloadRelease();
    void onDownloadToCurrentDir();
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void onDownloadFinished(const QString &filePath, bool success);
    void onDownloadError(const QString &error);
    void onAppImagesScanned(const QList<AppImageInfo> &images);
    void onSyncFromHome();
    void onSyncToHome();
    void onSyncProgress(const QString &message);
    void onSyncFinished(bool success, const QString &message);
    void onSyncError(const QString &error);
    void onTrayShowWindow();
    void onTrayQuit();
    void onTrayLaunchVersion(const AppImageInfo &appImage);
    void onLaunchRow(int row);
    void onDeleteRow(int row);
    void onLaunchFromButton();
    void onDeleteFromButton();
    void deleteAppImageByPath(const QString &path);
    void onLaunchLastClicked();
    void onTrayLaunchLast();

protected:
    void changeEvent(QEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private:
    void setupUI();
    void setupLaunchTab();
    void setupGitHubTab();
    void retranslateUi();
    void setApplicationLanguage(const QString &localeCode, bool sendLanguageChangeEvent = true);
    void onLanguageChanged(int index);
    void saveLastLaunched(const QString &filePath);
    QString getLastLaunched() const;
    void updateLaunchLastButton();
    void launchAppImageByPath(const QString &filePath);

private:
    QTranslator *m_translator;
    QComboBox *m_languageCombo;
    QLabel *m_titleLabel;
    QLabel *m_languageLabel;
    QPushButton *m_browseButton;
    QGroupBox *m_listGroup;
    QGroupBox *m_infoGroup;
    QGroupBox *m_presetGroup;
    QLabel *m_configLabel;
    QGroupBox *m_paramsGroup;
    QGroupBox *m_logGroup;
    QLabel *m_githubTitleLabel;

private:
    void updateReleasesTable();
    void updateReleasesDownloadStatus();
    AppImageAsset selectAppImageForRelease(const GitHubRelease &release);
    QString formatFileSize(qint64 size) const;

    QTabWidget *m_tabWidget;
    QWidget *m_launchTab;
    QWidget *m_githubTab;
    QTableWidget *m_appImageTable;
    QLabel *m_versionLabel;
    QLabel *m_fileSizeLabel;
    QLabel *m_lastModifiedLabel;
    QLineEdit *m_paramsEdit;
    QPushButton *m_scanButton;
    QPushButton *m_launchButton;
    QPushButton *m_launchWithParamsButton;
    QPushButton *m_launchLastButton;
    QPushButton *m_syncFromHomeButton;
    QPushButton *m_syncToHomeButton;
    QTextEdit *m_logOutput;
    QComboBox *m_configPreset;
    QTableWidget *m_releasesTable;
    ReleasesTableController *m_releasesTableController;
    QPushButton *m_refreshReleasesButton;
    QPushButton *m_downloadButton;
    QPushButton *m_downloadToCurrentDirButton;
    QProgressBar *m_downloadProgress;
    QLabel *m_githubStatusLabel;
    IDownloader *m_downloader;
    IAppImageManager *m_appImageManager;
    IConfigSync *m_configSync;
    ITrayManager *m_trayManager;
    QList<AppImageInfo> m_appImages;
    AppImageInfo m_selectedAppImage;
    QList<GitHubRelease> m_githubReleases;
    int m_downloadRowIndex;
    QString m_scriptDir;
};

#endif
