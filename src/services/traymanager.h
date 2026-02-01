#ifndef TRAYMANAGER_H
#define TRAYMANAGER_H

#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include "../interfaces/itraymanager.h"

class QWidget;

class TrayManager : public ITrayManager
{
    Q_OBJECT
public:
    explicit TrayManager(QObject *parent = nullptr);
    ~TrayManager();

    void setup(QWidget *mainWindow) override;
    void updateVersionsMenu(const QList<AppImageInfo> &appImages) override;
    void updateLastLaunched(const QString &filePath) override;
    bool isAvailable() const override;
    bool isVisible() const override;

private slots:
    void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void onShowWindow();
    void onQuitApplication();
    void onLaunchFromTray();
    void onLaunchLastFromTray();

private:
    void setupTrayIcon();
    void createMenu();

    QSystemTrayIcon *m_trayIcon;
    QMenu *m_trayMenu;
    QAction *m_showAction;
    QAction *m_quitAction;
    QAction *m_launchLastAction;
    QMenu *m_versionsMenu;
    QList<QAction*> m_versionActions;
    QWidget *m_mainWindow;
    QString m_lastLaunchedPath;
};

#endif
