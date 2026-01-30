#ifndef APP_VERSION
#define APP_VERSION "1.0.2"
#endif
#include <QApplication>
#include "helpers/iconhelper.h"
#include "mainwindow.h"
#include "types.h"
#include "services/downloader.h"
#include "services/appimagemanager.h"
#include "services/configsync.h"
#include "services/traymanager.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    qRegisterMetaType<AppImageInfo>("AppImageInfo");

    app.setApplicationName("FreeCAD Launcher");
    app.setApplicationVersion(APP_VERSION);
    app.setOrganizationName("FreeCAD Tools");

    QIcon appIcon = IconHelper::applicationIcon();
    if (!appIcon.isNull()) {
        app.setWindowIcon(appIcon);
    }

    IDownloader *downloader = new Downloader(&app);
    IAppImageManager *appImageManager = new AppImageManager(&app);
    IConfigSync *configSync = new ConfigSync(&app);
    ITrayManager *trayManager = new TrayManager(&app);

    MainWindow window(downloader, appImageManager, configSync, trayManager);
    window.show();

    return app.exec();
}
