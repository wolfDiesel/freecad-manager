#include "traymanager.h"
#include "../helpers/iconhelper.h"
#include <QApplication>
#include <QVariant>
#include <QFileInfo>
#include <QFile>

TrayManager::TrayManager(QObject *parent)
    : ITrayManager(parent)
    , m_trayIcon(nullptr)
    , m_trayMenu(nullptr)
    , m_showAction(nullptr)
    , m_quitAction(nullptr)
    , m_launchLastAction(nullptr)
    , m_versionsMenu(nullptr)
    , m_mainWindow(nullptr)
{
}

TrayManager::~TrayManager()
{
    if (m_trayIcon) {
        m_trayIcon->hide();
    }
}

void TrayManager::setup(QWidget *mainWindow)
{
    m_mainWindow = mainWindow;
    
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        return;
    }
    
    setupTrayIcon();
    createMenu();
    
    if (m_trayIcon) {
        m_trayIcon->show();
    }
}

void TrayManager::setupTrayIcon()
{
    m_trayIcon = new QSystemTrayIcon(this);

    QIcon trayIcon = IconHelper::applicationIcon();
    if (trayIcon.isNull()) {
        trayIcon = QIcon::fromTheme("application-x-executable");
    }

    m_trayIcon->setIcon(trayIcon);
    m_trayIcon->setToolTip(tr("FreeCAD Launcher"));
    
    connect(m_trayIcon, &QSystemTrayIcon::activated, this, &TrayManager::onTrayIconActivated);
}

void TrayManager::createMenu()
{
    if (!m_trayIcon) {
        return;
    }
    
    m_trayMenu = new QMenu(m_mainWindow);
    
    m_showAction = new QAction(tr("Show window"), this);
    connect(m_showAction, &QAction::triggered, this, &TrayManager::onShowWindow);
    m_trayMenu->addAction(m_showAction);
    
    m_trayMenu->addSeparator();
    
    m_launchLastAction = new QAction(tr("Launch last"), this);
    m_launchLastAction->setEnabled(false);
    connect(m_launchLastAction, &QAction::triggered, this, &TrayManager::onLaunchLastFromTray);
    m_trayMenu->addAction(m_launchLastAction);
    
    m_versionsMenu = new QMenu(tr("Launch version"), m_mainWindow);
    m_trayMenu->addMenu(m_versionsMenu);
    
    m_trayMenu->addSeparator();
    
    m_quitAction = new QAction(tr("Quit launcher"), this);
    connect(m_quitAction, &QAction::triggered, this, &TrayManager::onQuitApplication);
    m_trayMenu->addAction(m_quitAction);
    
    m_trayIcon->setContextMenu(m_trayMenu);
}

void TrayManager::updateVersionsMenu(const QList<AppImageInfo> &appImages)
{
    if (!m_versionsMenu) {
        return;
    }
    
    for (QAction *action : m_versionActions) {
        m_versionsMenu->removeAction(action);
        delete action;
    }
    m_versionActions.clear();
    
    if (appImages.isEmpty()) {
        QAction *noVersionsAction = new QAction(tr("No versions available"), this);
        noVersionsAction->setEnabled(false);
        m_versionsMenu->addAction(noVersionsAction);
        m_versionActions.append(noVersionsAction);
        return;
    }
    
    for (const AppImageInfo &info : appImages) {
        QString displayText = info.fileName;
        if (!info.version.isEmpty()) {
            displayText = QString("v%1 - %2").arg(info.version, info.fileName);
        }
        
        QAction *versionAction = new QAction(displayText, this);
        versionAction->setData(QVariant::fromValue(info));
        connect(versionAction, &QAction::triggered, this, &TrayManager::onLaunchFromTray);
        m_versionsMenu->addAction(versionAction);
        m_versionActions.append(versionAction);
    }
}

bool TrayManager::isAvailable() const
{
    return QSystemTrayIcon::isSystemTrayAvailable();
}

bool TrayManager::isVisible() const
{
    return m_trayIcon && m_trayIcon->isVisible();
}

void TrayManager::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::DoubleClick) {
        onShowWindow();
    }
}

void TrayManager::onShowWindow()
{
    emit showWindowRequested();
}

void TrayManager::onQuitApplication()
{
    emit quitRequested();
}

void TrayManager::onLaunchFromTray()
{
    QAction *action = qobject_cast<QAction*>(sender());
    if (!action) {
        return;
    }
    
    AppImageInfo info = action->data().value<AppImageInfo>();
    if (info.filePath.isEmpty()) {
        return;
    }
    
    emit launchVersionRequested(info);
}

void TrayManager::onLaunchLastFromTray()
{
    emit launchLastRequested();
}

void TrayManager::updateLastLaunched(const QString &filePath)
{
    m_lastLaunchedPath = filePath;
    if (!m_launchLastAction) return;
    
    if (filePath.isEmpty()) {
        m_launchLastAction->setText(tr("Launch last"));
        m_launchLastAction->setEnabled(false);
    } else {
        QString fileName = QFileInfo(filePath).fileName();
        m_launchLastAction->setText(tr("Launch last (%1)").arg(fileName));
        m_launchLastAction->setEnabled(QFile::exists(filePath));
    }
}
