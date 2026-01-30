#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QScrollBar>
#include <QFileDialog>
#include <QFile>
#include <QFileInfo>
#include <QCoreApplication>
#include <QApplication>
#include <QRegularExpression>
#include <QMessageBox>
#include <QSettings>
#include <QDir>
#include <QLocale>
#include <algorithm>
#include "utils/filesizeformatter.h"
#include "ui/appimageselectordialog.h"
#include "ui/releasestablecontroller.h"
#include "constants.h"

MainWindow::MainWindow(IDownloader *downloader, IAppImageManager *appImageManager,
                       IConfigSync *configSync, ITrayManager *trayManager,
                       QWidget *parent)
    : QMainWindow(parent)
    , m_translator(new QTranslator(this))
    , m_languageCombo(nullptr)
    , m_titleLabel(nullptr)
    , m_languageLabel(nullptr)
    , m_browseButton(nullptr)
    , m_listGroup(nullptr)
    , m_infoGroup(nullptr)
    , m_presetGroup(nullptr)
    , m_configLabel(nullptr)
    , m_paramsGroup(nullptr)
    , m_logGroup(nullptr)
    , m_githubTitleLabel(nullptr)
    , m_downloader(downloader)
    , m_appImageManager(appImageManager)
    , m_configSync(configSync)
    , m_trayManager(trayManager)
    , m_releasesTableController(nullptr)
    , m_downloadRowIndex(-1)
{
    const QByteArray appImagePath = qgetenv("APPIMAGE");
    if (!appImagePath.isEmpty()) {
        m_scriptDir = QFileInfo(QFile::decodeName(appImagePath)).absolutePath();
    } else {
        m_scriptDir = QCoreApplication::applicationDirPath();
    }
    QString savedLang = QSettings().value(QStringLiteral("language"), QString()).toString();
    setApplicationLanguage(savedLang.isEmpty() ? QLocale::system().name() : savedLang, false);
    m_downloader->setWorkingDirectory(m_scriptDir);
    m_appImageManager->setWorkingDirectory(m_scriptDir);

    connect(m_configSync, &IConfigSync::syncProgress, this, &MainWindow::onSyncProgress);
    connect(m_configSync, &IConfigSync::syncFinished, this, &MainWindow::onSyncFinished);
    connect(m_configSync, &IConfigSync::syncError, this, &MainWindow::onSyncError);

    connect(m_downloader, &IDownloader::releasesFetched, this, &MainWindow::onReleasesFetched);
    connect(m_downloader, &IDownloader::fetchError, this, &MainWindow::onFetchError);
    connect(m_downloader, &IDownloader::downloadProgress, this, &MainWindow::onDownloadProgress);
    connect(m_downloader, &IDownloader::downloadFinished, this, &MainWindow::onDownloadFinished);
    connect(m_downloader, &IDownloader::downloadError, this, &MainWindow::onDownloadError);

    connect(m_appImageManager, &IAppImageManager::appImagesScanned, this, &MainWindow::onAppImagesScanned);
    connect(m_appImageManager, &IAppImageManager::processFinished, this, &MainWindow::onProcessFinished);
    connect(m_appImageManager, &IAppImageManager::processError, this, &MainWindow::onProcessError);
    connect(m_appImageManager, &IAppImageManager::processOutput, [this](const QString &output) {
        m_logOutput->append(output);
    });

    connect(m_trayManager, &ITrayManager::showWindowRequested, this, &MainWindow::onTrayShowWindow);
    connect(m_trayManager, &ITrayManager::quitRequested, this, &MainWindow::onTrayQuit);
    connect(m_trayManager, &ITrayManager::launchVersionRequested, this, &MainWindow::onTrayLaunchVersion);

    setupUI();
    m_trayManager->setup(this);
    onScanAppImages();
}

MainWindow::~MainWindow()
{
    delete m_releasesTableController;
}

void MainWindow::setupUI()
{
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    QHBoxLayout *headerLayout = new QHBoxLayout();
    m_titleLabel = new QLabel(tr("FreeCAD Launcher"), this);
    m_titleLabel->setStyleSheet("font-size: 18px; font-weight: bold;");
    headerLayout->addWidget(m_titleLabel);
    headerLayout->addStretch();
    m_languageCombo = new QComboBox(this);
    m_languageCombo->addItem(tr("System"), QString());
    m_languageCombo->addItem(QStringLiteral("English"), QStringLiteral("en"));
    m_languageCombo->addItem(QStringLiteral("Русский"), QStringLiteral("ru"));
    m_languageCombo->addItem(QStringLiteral("Français"), QStringLiteral("fr"));
    QString savedLang = QSettings().value(QStringLiteral("language"), QString()).toString();
    int idx = savedLang.isEmpty() ? 0 : m_languageCombo->findData(savedLang);
    if (idx < 0) idx = 1;
    m_languageCombo->setCurrentIndex(idx);
    m_languageLabel = new QLabel(tr("Language:"), this);
    headerLayout->addWidget(m_languageLabel);
    headerLayout->addWidget(m_languageCombo);
    mainLayout->addLayout(headerLayout);

    m_tabWidget = new QTabWidget(this);
    mainLayout->addWidget(m_tabWidget);

    setupLaunchTab();
    setupGitHubTab();

    connect(m_languageCombo, &QComboBox::currentIndexChanged, this, [this](int index) { onLanguageChanged(index); });

    setWindowTitle(tr("FreeCAD Launcher"));
    resize(700, 800);
}

void MainWindow::setupLaunchTab()
{
    m_launchTab = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(m_launchTab);
    
    QHBoxLayout *scanLayout = new QHBoxLayout();
    m_scanButton = new QPushButton(tr("Refresh list"), this);
    m_browseButton = new QPushButton(tr("Choose directory..."), this);
    scanLayout->addWidget(m_scanButton);
    scanLayout->addWidget(m_browseButton);
    scanLayout->addStretch();
    mainLayout->addLayout(scanLayout);

    connect(m_scanButton, &QPushButton::clicked, this, &MainWindow::onScanAppImages);
    connect(m_browseButton, &QPushButton::clicked, [this]() {
        QString dir = QFileDialog::getExistingDirectory(this, tr("Choose directory with AppImage files"), m_scriptDir);
        if (!dir.isEmpty()) {
            m_scriptDir = dir;
            m_downloader->setWorkingDirectory(m_scriptDir);
            m_appImageManager->setWorkingDirectory(m_scriptDir);
            onScanAppImages();
            updateReleasesDownloadStatus();
        }
    });
    
    m_listGroup = new QGroupBox(tr("Available FreeCAD versions"), this);
    QVBoxLayout *listLayout = new QVBoxLayout();
    m_appImageList = new QListWidget(this);
    m_appImageList->setSelectionMode(QAbstractItemView::SingleSelection);
    listLayout->addWidget(m_appImageList);
    m_listGroup->setLayout(listLayout);
    mainLayout->addWidget(m_listGroup);
    
    connect(m_appImageList, &QListWidget::itemSelectionChanged, this, &MainWindow::onAppImageSelected);
    connect(m_appImageList, &QListWidget::itemDoubleClicked, this, &MainWindow::onLaunchClicked);
    
    m_infoGroup = new QGroupBox(tr("Information"), this);
    QVBoxLayout *infoLayout = new QVBoxLayout();
    m_versionLabel = new QLabel(tr("Version: -"), this);
    m_fileSizeLabel = new QLabel(tr("Size: -"), this);
    m_lastModifiedLabel = new QLabel(tr("Modified: -"), this);
    infoLayout->addWidget(m_versionLabel);
    infoLayout->addWidget(m_fileSizeLabel);
    infoLayout->addWidget(m_lastModifiedLabel);
    m_infoGroup->setLayout(infoLayout);
    mainLayout->addWidget(m_infoGroup);
    
    m_presetGroup = new QGroupBox(tr("Configuration preset"), this);
    QVBoxLayout *presetLayout = new QVBoxLayout();

    QHBoxLayout *presetComboLayout = new QHBoxLayout();
    m_configPreset = new QComboBox(this);
    m_configPreset->addItem(tr("Portable (fcad/)"), "portable");
    m_configPreset->addItem(tr("System (~/.config/FreeCAD)"), "system");
    m_configLabel = new QLabel(tr("Configuration:"), this);
    presetComboLayout->addWidget(m_configLabel);
    presetComboLayout->addWidget(m_configPreset);
    presetComboLayout->addStretch();
    presetLayout->addLayout(presetComboLayout);
    
    QHBoxLayout *syncLayout = new QHBoxLayout();
    m_syncFromHomeButton = new QPushButton(tr("Sync from home directory"), this);
    m_syncToHomeButton = new QPushButton(tr("Sync to home directory"), this);
    syncLayout->addWidget(m_syncFromHomeButton);
    syncLayout->addWidget(m_syncToHomeButton);
    syncLayout->addStretch();
    presetLayout->addLayout(syncLayout);
    
    connect(m_syncFromHomeButton, &QPushButton::clicked, this, &MainWindow::onSyncFromHome);
    connect(m_syncToHomeButton, &QPushButton::clicked, this, &MainWindow::onSyncToHome);
    
    m_presetGroup->setLayout(presetLayout);
    mainLayout->addWidget(m_presetGroup);

    m_paramsGroup = new QGroupBox(tr("Additional launch parameters"), this);
    QVBoxLayout *paramsLayout = new QVBoxLayout();
    m_paramsEdit = new QLineEdit(this);
    m_paramsEdit->setPlaceholderText(tr("e.g. --console --run-test 0"));
    paramsLayout->addWidget(m_paramsEdit);
    m_paramsGroup->setLayout(paramsLayout);
    mainLayout->addWidget(m_paramsGroup);
    
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    m_launchButton = new QPushButton(tr("Launch"), this);
    m_launchButton->setStyleSheet("font-weight: bold; padding: 8px;");
    m_launchWithParamsButton = new QPushButton(tr("Launch with parameters"), this);
    m_launchWithParamsButton->setStyleSheet("padding: 8px;");
    int buttonWidth = qMax(m_launchButton->sizeHint().width(), m_launchWithParamsButton->sizeHint().width());
    m_launchButton->setMinimumWidth(buttonWidth);
    m_launchWithParamsButton->setMinimumWidth(buttonWidth);
    buttonLayout->addWidget(m_launchButton);
    buttonLayout->addWidget(m_launchWithParamsButton);
    buttonLayout->addStretch();
    mainLayout->addLayout(buttonLayout);
    
    connect(m_launchButton, &QPushButton::clicked, this, &MainWindow::onLaunchClicked);
    connect(m_launchWithParamsButton, &QPushButton::clicked, this, &MainWindow::onLaunchWithParamsClicked);
    
    m_logGroup = new QGroupBox(tr("Launch log"), this);
    QVBoxLayout *logLayout = new QVBoxLayout();
    m_logOutput = new QTextEdit(this);
    m_logOutput->setReadOnly(true);
    m_logOutput->setMaximumHeight(150);
    logLayout->addWidget(m_logOutput);
    m_logGroup->setLayout(logLayout);
    mainLayout->addWidget(m_logGroup);

    m_tabWidget->addTab(m_launchTab, tr("Launch"));
}

void MainWindow::setupGitHubTab()
{
    m_githubTab = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(m_githubTab);
    
    QHBoxLayout *headerLayout = new QHBoxLayout();
    m_githubTitleLabel = new QLabel(tr("FreeCAD releases from GitHub"), this);
    m_githubTitleLabel->setStyleSheet("font-size: 16px; font-weight: bold;");
    m_refreshReleasesButton = new QPushButton(tr("Refresh list"), this);
    headerLayout->addWidget(m_githubTitleLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(m_refreshReleasesButton);
    mainLayout->addLayout(headerLayout);
    
    connect(m_refreshReleasesButton, &QPushButton::clicked, this, &MainWindow::onFetchGitHubReleases);
    
    m_releasesTable = new QTableWidget(this);
    m_releasesTable->setColumnCount(5);
    m_releasesTable->setHorizontalHeaderLabels(QStringList() << tr("Version") << tr("Name") << tr("Date") << tr("Size") << tr("Downloaded"));
    m_releasesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_releasesTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_releasesTable->horizontalHeader()->setStretchLastSection(false);
    m_releasesTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    m_releasesTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mainLayout->addWidget(m_releasesTable);
    
    m_githubStatusLabel = new QLabel(tr("Click 'Refresh list' to load releases"), this);
    mainLayout->addWidget(m_githubStatusLabel);
    
    m_downloadProgress = new QProgressBar(this);
    m_downloadProgress->setVisible(false);
    mainLayout->addWidget(m_downloadProgress);
    
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    m_downloadToCurrentDirButton = new QPushButton(tr("Download to working directory"), this);
    m_downloadToCurrentDirButton->setStyleSheet("font-weight: bold; padding: 8px;");
    m_downloadToCurrentDirButton->setEnabled(false);
    m_downloadButton = new QPushButton(tr("Download to other folder..."), this);
    m_downloadButton->setEnabled(false);
    buttonLayout->addWidget(m_downloadToCurrentDirButton);
    buttonLayout->addWidget(m_downloadButton);
    buttonLayout->addStretch();
    mainLayout->addLayout(buttonLayout);
    
    connect(m_releasesTable, &QTableWidget::itemDoubleClicked, [this](QTableWidgetItem *) {
        if (!m_downloader->isDownloading()) {
            onDownloadToCurrentDir();
        } else {
            QMessageBox::information(this, tr("Download in progress"), 
                tr("Please wait for the current download to finish."));
        }
    });
    connect(m_downloadToCurrentDirButton, &QPushButton::clicked, this, &MainWindow::onDownloadToCurrentDir);
    connect(m_downloadButton, &QPushButton::clicked, this, &MainWindow::onDownloadRelease);
    connect(m_releasesTable, &QTableWidget::itemSelectionChanged, [this]() {
        bool hasSelection = m_releasesTable->currentRow() >= 0;
        bool isDownloading = m_downloader->isDownloading();
        m_downloadButton->setEnabled(hasSelection && !isDownloading);
        m_downloadToCurrentDirButton->setEnabled(hasSelection && !isDownloading);
    });
    m_releasesTableController = new ReleasesTableController(m_releasesTable, m_downloader);
    m_tabWidget->addTab(m_githubTab, tr("GitHub Releases"));
}

void MainWindow::setApplicationLanguage(const QString &localeCode, bool sendLanguageChangeEvent)
{
    QCoreApplication::removeTranslator(m_translator);
    QString localeName = localeCode.isEmpty() ? QLocale::system().name() : localeCode;
    if (!localeName.startsWith(QLatin1String("en"))) {
        QString lang = localeName.split(QLatin1Char('_')).first();
        if (lang.isEmpty()) lang = localeName.left(2);
        QString baseName = QLatin1String("FreeCADLauncher_") + lang;
        QStringList searchPaths;
        searchPaths << (QCoreApplication::applicationDirPath() + QDir::separator() + QStringLiteral("translations"));
        searchPaths << (QCoreApplication::applicationDirPath() + QDir::separator() + QStringLiteral("..") + QDir::separator() + QStringLiteral("translations"));
        for (const QString &path : searchPaths) {
            if (m_translator->load(baseName, path)) {
                QCoreApplication::installTranslator(m_translator);
                break;
            }
        }
    }
    if (sendLanguageChangeEvent) {
        QEvent ev(QEvent::LanguageChange);
        QCoreApplication::sendEvent(this, &ev);
    }
}

void MainWindow::retranslateUi()
{
    setWindowTitle(tr("FreeCAD Launcher"));
    if (m_titleLabel) m_titleLabel->setText(tr("FreeCAD Launcher"));
    if (m_languageLabel) m_languageLabel->setText(tr("Language:"));
    if (m_languageCombo) {
        m_languageCombo->blockSignals(true);
        m_languageCombo->setItemText(0, tr("System"));
        m_languageCombo->blockSignals(false);
    }
    if (m_scanButton) m_scanButton->setText(tr("Refresh list"));
    if (m_browseButton) m_browseButton->setText(tr("Choose directory..."));
    if (m_listGroup) m_listGroup->setTitle(tr("Available FreeCAD versions"));
    if (m_infoGroup) m_infoGroup->setTitle(tr("Information"));
    if (m_versionLabel) m_versionLabel->setText(tr("Version: -"));
    if (m_fileSizeLabel) m_fileSizeLabel->setText(tr("Size: -"));
    if (m_lastModifiedLabel) m_lastModifiedLabel->setText(tr("Modified: -"));
    if (m_presetGroup) m_presetGroup->setTitle(tr("Configuration preset"));
    if (m_configLabel) m_configLabel->setText(tr("Configuration:"));
    if (m_configPreset) {
        m_configPreset->setItemText(0, tr("Portable (fcad/)"));
        m_configPreset->setItemText(1, tr("System (~/.config/FreeCAD)"));
    }
    if (m_syncFromHomeButton) m_syncFromHomeButton->setText(tr("Sync from home directory"));
    if (m_syncToHomeButton) m_syncToHomeButton->setText(tr("Sync to home directory"));
    if (m_paramsGroup) m_paramsGroup->setTitle(tr("Additional launch parameters"));
    if (m_paramsEdit) m_paramsEdit->setPlaceholderText(tr("e.g. --console --run-test 0"));
    if (m_launchButton) m_launchButton->setText(tr("Launch"));
    if (m_launchWithParamsButton) m_launchWithParamsButton->setText(tr("Launch with parameters"));
    if (m_logGroup) m_logGroup->setTitle(tr("Launch log"));
    if (m_tabWidget) m_tabWidget->setTabText(0, tr("Launch"));
    if (m_githubTitleLabel) m_githubTitleLabel->setText(tr("FreeCAD releases from GitHub"));
    if (m_refreshReleasesButton) m_refreshReleasesButton->setText(tr("Refresh list"));
    if (m_releasesTable) {
        m_releasesTable->setHorizontalHeaderLabels(QStringList() << tr("Version") << tr("Name") << tr("Date") << tr("Size") << tr("Downloaded"));
    }
    if (m_githubStatusLabel) m_githubStatusLabel->setText(tr("Click 'Refresh list' to load releases"));
    if (m_downloadToCurrentDirButton) m_downloadToCurrentDirButton->setText(tr("Download to working directory"));
    if (m_downloadButton) m_downloadButton->setText(tr("Download to other folder..."));
    if (m_tabWidget) m_tabWidget->setTabText(1, tr("GitHub Releases"));
    if (m_releasesTableController && !m_githubReleases.isEmpty())
        updateReleasesDownloadStatus();
}

void MainWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    QMainWindow::changeEvent(event);
}

void MainWindow::onLanguageChanged(int index)
{
    if (index < 0 || !m_languageCombo) return;
    QString code = m_languageCombo->itemData(index).toString();
    QString toApply = code.isEmpty() ? QLocale::system().name() : code;
    QSettings().setValue(QStringLiteral("language"), code);
    m_languageCombo->blockSignals(true);
    setApplicationLanguage(toApply);
    retranslateUi();
    m_languageCombo->blockSignals(false);
    updateReleasesDownloadStatus();
}

void MainWindow::onScanAppImages()
{
    m_appImages = m_appImageManager->scanAppImages();
}

void MainWindow::onAppImagesScanned(const QList<AppImageInfo> &images)
{
    m_appImages = images;
    std::sort(m_appImages.begin(), m_appImages.end(), [](const AppImageInfo &a, const AppImageInfo &b) {
        return a.lastModified > b.lastModified;
    });
    m_appImageList->clear();
    for (int i = 0; i < m_appImages.size(); ++i) {
        const AppImageInfo &info = m_appImages[i];
        QString displayText = info.fileName;
        if (!info.version.isEmpty()) {
            displayText += QString(" (v%1)").arg(info.version);
        }
        m_appImageList->addItem(displayText);
        QWidget *row = new QWidget(this);
        QHBoxLayout *h = new QHBoxLayout(row);
        h->setContentsMargins(4, 2, 4, 2);
        h->setSpacing(8);
        QLabel *lbl = new QLabel(displayText, row);
        lbl->setTextInteractionFlags(Qt::NoTextInteraction);
        QPushButton *launchBtn = new QPushButton(tr("Launch"), row);
        QPushButton *deleteBtn = new QPushButton(tr("Delete"), row);
        h->addWidget(lbl, 1);
        h->addWidget(launchBtn);
        h->addWidget(deleteBtn);
        int rowIndex = i;
        connect(launchBtn, &QPushButton::clicked, this, [this, rowIndex]() { onLaunchRow(rowIndex); });
        connect(deleteBtn, &QPushButton::clicked, this, [this, rowIndex]() { onDeleteRow(rowIndex); });
        m_appImageList->setItemWidget(m_appImageList->item(i), row);
    }
    if (m_appImages.isEmpty()) {
        m_appImageList->addItem(tr("No AppImage files found"));
        m_appImageList->item(0)->setFlags(Qt::NoItemFlags);
    }
    m_logOutput->append(tr("Found %1 AppImage file(s)").arg(m_appImages.size()));
    updateReleasesDownloadStatus();
    m_trayManager->updateVersionsMenu(m_appImages);
}

void MainWindow::onAppImageSelected()
{
    int currentRow = m_appImageList->currentRow();
    if (currentRow < 0 || currentRow >= m_appImages.size()) {
        m_versionLabel->setText(tr("Version: -"));
        m_fileSizeLabel->setText(tr("Size: -"));
        m_lastModifiedLabel->setText(tr("Modified: -"));
        m_selectedAppImage = AppImageInfo();
        return;
    }
    
    m_selectedAppImage = m_appImages[currentRow];
    
    m_versionLabel->setText(tr("Version: %1").arg(
        m_selectedAppImage.version.isEmpty() ? tr("unknown") : m_selectedAppImage.version));
    m_fileSizeLabel->setText(tr("Size: %1").arg(formatFileSize(m_selectedAppImage.fileSize)));
    m_lastModifiedLabel->setText(tr("Modified: %1").arg(
        m_selectedAppImage.lastModified.toString("yyyy-MM-dd HH:mm:ss")));
}

void MainWindow::onLaunchRow(int row)
{
    if (row < 0 || row >= m_appImages.size()) return;
    m_appImageList->setCurrentRow(row);
    m_selectedAppImage = m_appImages[row];
    if (m_appImageManager->isRunning()) {
        QMessageBox::information(this, tr("Information"), tr("FreeCAD is already running"));
        return;
    }
    m_appImageManager->setConfigPreset(m_configPreset->currentData().toString());
    m_logOutput->clear();
    m_logOutput->append(tr("Launch: %1").arg(m_selectedAppImage.filePath));
    m_logOutput->append(tr("Configuration: %1").arg(m_configPreset->currentData().toString()));
    m_logOutput->append("---");
    m_appImageManager->launchFreeCAD(m_selectedAppImage);
    m_launchButton->setEnabled(false);
    m_launchWithParamsButton->setEnabled(false);
}

void MainWindow::onDeleteRow(int row)
{
    if (row < 0 || row >= m_appImages.size()) return;
    const QString path = m_appImages[row].filePath;
    QString fileName = QFileInfo(path).fileName();
    int ret = QMessageBox::question(this, tr("Delete file"),
        tr("Delete %1?").arg(fileName),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (ret != QMessageBox::Yes) return;
    if (QFile::remove(path)) {
        onScanAppImages();
    } else {
        QMessageBox::warning(this, tr("Error"), tr("Could not delete file"));
    }
}

void MainWindow::onLaunchClicked()
{
    if (m_selectedAppImage.filePath.isEmpty()) {
        QMessageBox::warning(this, tr("Error"), tr("Select an AppImage file to launch"));
        return;
    }
    if (m_appImageManager->isRunning()) {
        QMessageBox::information(this, tr("Information"), tr("FreeCAD is already running"));
        return;
    }
    m_appImageManager->setConfigPreset(m_configPreset->currentData().toString());
    m_logOutput->clear();
    m_logOutput->append(tr("Launch: %1").arg(m_selectedAppImage.filePath));
    m_logOutput->append(tr("Configuration: %1").arg(m_configPreset->currentData().toString()));
    m_logOutput->append("---");
    m_appImageManager->launchFreeCAD(m_selectedAppImage);
    m_launchButton->setEnabled(false);
    m_launchWithParamsButton->setEnabled(false);
}

void MainWindow::onLaunchWithParamsClicked()
{
    if (m_selectedAppImage.filePath.isEmpty()) {
        QMessageBox::warning(this, tr("Error"), tr("Select an AppImage file to launch"));
        return;
    }
    
    QString paramsStr = m_paramsEdit->text().trimmed();
    QStringList params;
    
    if (!paramsStr.isEmpty()) {
        params = paramsStr.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    }
    
    if (m_appImageManager->isRunning()) {
        QMessageBox::information(this, tr("Information"), tr("FreeCAD is already running"));
        return;
    }
    
    m_appImageManager->setConfigPreset(m_configPreset->currentData().toString());
    
    m_logOutput->clear();
    m_logOutput->append(tr("Launch: %1").arg(m_selectedAppImage.filePath));
    if (!params.isEmpty()) {
        m_logOutput->append(tr("Parameters: %1").arg(params.join(" ")));
    }
    m_logOutput->append(tr("Configuration: %1").arg(m_configPreset->currentData().toString()));
    m_logOutput->append("---");
    
    m_appImageManager->launchFreeCAD(m_selectedAppImage, params);
    m_launchButton->setEnabled(false);
    m_launchWithParamsButton->setEnabled(false);
}

void MainWindow::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    m_launchButton->setEnabled(true);
    m_launchWithParamsButton->setEnabled(true);
    
    if (exitStatus == QProcess::NormalExit) {
        m_logOutput->append(tr("FreeCAD exited with code: %1").arg(exitCode));
    } else {
        m_logOutput->append(tr("FreeCAD crashed"));
    }
    
    QScrollBar *scrollBar = m_logOutput->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}

void MainWindow::onProcessError(QProcess::ProcessError error)
{
    m_launchButton->setEnabled(true);
    m_launchWithParamsButton->setEnabled(true);
    
    QString errorMsg;
    switch (error) {
        case QProcess::FailedToStart:
            errorMsg = tr("Failed to start process");
            break;
        case QProcess::Crashed:
            errorMsg = tr("Process crashed");
            break;
        case QProcess::Timedout:
            errorMsg = tr("Process timeout");
            break;
        case QProcess::WriteError:
            errorMsg = tr("Write error");
            break;
        case QProcess::ReadError:
            errorMsg = tr("Read error");
            break;
        default:
            errorMsg = tr("Unknown error");
    }
    
    m_logOutput->append(tr("ERROR: %1").arg(errorMsg));
    QMessageBox::critical(this, tr("Error"), errorMsg);
}

void MainWindow::onFetchGitHubReleases()
{
    if (m_downloader->isDownloading()) {
        QMessageBox::information(this, tr("Download in progress"), 
            tr("Please wait for the download to finish before refreshing the release list."));
        return;
    }
    
    m_githubStatusLabel->setText(tr("Loading release list..."));
    m_refreshReleasesButton->setEnabled(false);
    m_downloader->fetchReleases();
}

void MainWindow::onReleasesFetched(const QList<GitHubRelease> &releases)
{
    m_githubReleases = releases;
    m_refreshReleasesButton->setEnabled(true);
    updateReleasesTable();
    m_githubStatusLabel->setText(tr("Found %1 release(s)").arg(releases.size()));
}

void MainWindow::onFetchError(const QString &error)
{
    m_refreshReleasesButton->setEnabled(true);
    m_githubStatusLabel->setText(tr("Load error: %1").arg(error));
    QMessageBox::warning(this, tr("Error"), tr("Failed to load release list:\n%1").arg(error));
}

AppImageAsset MainWindow::selectAppImageForRelease(const GitHubRelease &release)
{
    if (release.appImages.size() == 1) {
        return release.appImages.first();
    }
    if (release.appImages.isEmpty()) {
        return AppImageAsset();
    }
    AppImageSelectorDialog dialog(release, this);
    if (dialog.exec() == QDialog::Accepted) {
        return dialog.selectedAppImage();
    }
    return AppImageAsset();
}

void MainWindow::updateReleasesTable()
{
    m_releasesTableController->updateTable(m_githubReleases);
}

void MainWindow::updateReleasesDownloadStatus()
{
    m_releasesTableController->updateDownloadStatus(m_githubReleases);
}

void MainWindow::onDownloadToCurrentDir()
{
    if (m_downloader->isDownloading()) {
        QMessageBox::information(this, tr("Download in progress"), 
            tr("Please wait for the current download to finish."));
        return;
    }
    
    int currentRow = m_releasesTable->currentRow();
    if (currentRow < 0 || currentRow >= m_githubReleases.size()) {
        QMessageBox::warning(this, tr("Error"), tr("Select a release to download"));
        return;
    }
    
    const GitHubRelease &release = m_githubReleases[currentRow];
    AppImageAsset selectedImage = selectAppImageForRelease(release);
    if (selectedImage.fileName.isEmpty()) {
        return;
    }
    QString savePath = m_scriptDir + "/" + selectedImage.fileName;
    
    if (QFile::exists(savePath)) {
        int ret = QMessageBox::question(this, tr("File already downloaded"), 
            tr("File %1 is already in the working directory.\nOverwrite?").arg(selectedImage.fileName),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (ret != QMessageBox::Yes) {
            return;
        }
    }
    
    m_downloadRowIndex = currentRow;
    m_githubStatusLabel->setText(tr("Downloading to working directory: %1").arg(m_scriptDir));
    m_downloader->downloadFile(selectedImage.downloadUrl, savePath);
}

void MainWindow::onDownloadRelease()
{
    if (m_downloader->isDownloading()) {
        QMessageBox::information(this, tr("Download in progress"), 
            tr("Please wait for the current download to finish."));
        return;
    }
    
    int currentRow = m_releasesTable->currentRow();
    if (currentRow < 0 || currentRow >= m_githubReleases.size()) {
        QMessageBox::warning(this, tr("Error"), tr("Select a release to download"));
        return;
    }
    
    const GitHubRelease &release = m_githubReleases[currentRow];
    AppImageAsset selectedImage = selectAppImageForRelease(release);
    if (selectedImage.fileName.isEmpty()) {
        return;
    }
    QString saveDir = QFileDialog::getExistingDirectory(this, tr("Choose directory to save to"), m_scriptDir);
    if (saveDir.isEmpty()) {
        return;
    }
    
    QString savePath = saveDir + "/" + selectedImage.fileName;
    
    if (QFile::exists(savePath)) {
        int ret = QMessageBox::question(this, tr("File exists"), 
            tr("File %1 already exists. Overwrite?").arg(selectedImage.fileName),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (ret != QMessageBox::Yes) {
            return;
        }
    }
    
    m_downloadRowIndex = currentRow;
    m_downloader->downloadFile(selectedImage.downloadUrl, savePath);
}

void MainWindow::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    if (bytesTotal > 0) {
        int progress = (bytesReceived * 100) / bytesTotal;
        m_downloadProgress->setValue(progress);
        m_downloadProgress->setVisible(true);
        m_githubStatusLabel->setText(tr("Downloading: %1% (%2 / %3 MB)")
            .arg(progress)
            .arg(bytesReceived / (1024.0 * 1024.0), 0, 'f', 1)
            .arg(bytesTotal / (1024.0 * 1024.0), 0, 'f', 1));
    }
}

void MainWindow::onDownloadFinished(const QString &filePath, bool success)
{
    m_downloadProgress->setVisible(false);
    m_downloadButton->setEnabled(true);
    m_downloadToCurrentDirButton->setEnabled(true);
    m_refreshReleasesButton->setEnabled(true);
    
    if (success) {
        m_githubStatusLabel->setText(tr("Download complete: %1").arg(QFileInfo(filePath).fileName()));
        QMessageBox::information(this, tr("Success"), tr("File saved:\n%1").arg(filePath));
        
        if (m_downloadRowIndex >= 0 && m_downloadRowIndex < m_releasesTable->rowCount()) {
            m_releasesTableController->updateDownloadStatus(m_githubReleases);
        }
        
        if (QFileInfo(filePath).absolutePath() == m_scriptDir) {
            onScanAppImages();
        }
    }
    
    m_downloadRowIndex = -1;
}

void MainWindow::onDownloadError(const QString &error)
{
    m_downloadProgress->setVisible(false);
    m_downloadButton->setEnabled(true);
    m_downloadToCurrentDirButton->setEnabled(true);
    m_refreshReleasesButton->setEnabled(true);
    m_githubStatusLabel->setText(tr("Download error: %1").arg(error));
    QMessageBox::critical(this, tr("Error"), tr("Failed to download file:\n%1").arg(error));
    m_downloadRowIndex = -1;
}

QString MainWindow::formatFileSize(qint64 size) const
{
    return FileSizeFormatter::format(size);
}

void MainWindow::onSyncFromHome()
{
    if (m_configPreset->currentData().toString() != "portable") {
        QMessageBox::information(this, tr("Information"), 
            tr("Sync is only available for portable configuration."));
        return;
    }
    
    int ret = QMessageBox::question(this, tr("Configuration sync"),
        tr("Copy configuration and data from home directory (~/.config/FreeCAD, ~/.local/share/FreeCAD)\n"
        "to portable configuration (fcad/)?\n\n"
        "Warning: existing files in portable configuration will be overwritten!"),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    
    if (ret != QMessageBox::Yes) {
        return;
    }
    
    QString portableConfigDir = PortableConfig::configDir(m_scriptDir);
    QString portableDataDir = PortableConfig::dataDir(m_scriptDir);
    m_logOutput->append("--- " + tr("Sync from home directory") + " ---");
    m_syncFromHomeButton->setEnabled(false);
    m_syncToHomeButton->setEnabled(false);
    
    bool success = m_configSync->syncFromHome(portableConfigDir, portableDataDir);
    
    m_syncFromHomeButton->setEnabled(true);
    m_syncToHomeButton->setEnabled(true);
    
    if (success) {
        m_logOutput->append(tr("Sync completed successfully"));
    }
}

void MainWindow::onSyncToHome()
{
    if (m_configPreset->currentData().toString() != "portable") {
        QMessageBox::information(this, tr("Information"), 
            tr("Sync is only available for portable configuration."));
        return;
    }
    
    int ret = QMessageBox::warning(this, tr("Sync to home directory"),
        tr("Contents of the portable folder (fcad/) will be copied to your home FreeCAD configuration:\n"
        "  • ~/.config/FreeCAD\n"
        "  • ~/.local/share/FreeCAD\n\n"
        "Existing files in these folders will be overwritten and cannot be recovered.\n\n"
        "Do you understand what you are doing? Continue?"),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    
    if (ret != QMessageBox::Yes) {
        return;
    }
    
    QString portableConfigDir = PortableConfig::configDir(m_scriptDir);
    QString portableDataDir = PortableConfig::dataDir(m_scriptDir);
    m_logOutput->append("--- " + tr("Sync to home directory") + " ---");
    m_syncFromHomeButton->setEnabled(false);
    m_syncToHomeButton->setEnabled(false);
    
    bool success = m_configSync->syncToHome(portableConfigDir, portableDataDir);
    
    m_syncFromHomeButton->setEnabled(true);
    m_syncToHomeButton->setEnabled(true);
    
    if (success) {
        m_logOutput->append(tr("Sync completed successfully"));
    }
}

void MainWindow::onSyncProgress(const QString &message)
{
    m_logOutput->append(message);
    QScrollBar *scrollBar = m_logOutput->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}

void MainWindow::onSyncFinished(bool success, const QString &message)
{
    if (success) {
        QMessageBox::information(this, tr("Success"), message);
    } else {
        QMessageBox::warning(this, tr("Error"), tr("Sync finished with errors:\n%1").arg(message));
    }
}

void MainWindow::onSyncError(const QString &error)
{
    m_logOutput->append(tr("ERROR: %1").arg(error));
    QMessageBox::critical(this, tr("Sync error"), error);
}

void MainWindow::onTrayShowWindow()
{
    show();
    raise();
    activateWindow();
}

void MainWindow::onTrayQuit()
{
    QApplication::quit();
}

void MainWindow::onTrayLaunchVersion(const AppImageInfo &appImage)
{
    if (appImage.filePath.isEmpty()) {
        return;
    }
    
    if (m_appImageManager->isRunning()) {
        return;
    }
    
    m_appImageManager->setConfigPreset(m_configPreset->currentData().toString());
    m_appImageManager->launchFreeCAD(appImage);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (m_trayManager && m_trayManager->isVisible()) {
        hide();
        event->ignore();
    } else {
        event->accept();
    }
}
