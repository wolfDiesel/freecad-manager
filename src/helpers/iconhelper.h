#ifndef ICONHELPER_H
#define ICONHELPER_H

#include <QIcon>
#include <QCoreApplication>
#include <QFile>
#include <QDir>

namespace IconHelper {
inline QIcon applicationIcon()
{
    QIcon icon;
    if (qEnvironmentVariableIsSet("APPIMAGE")) {
        const QString base = QCoreApplication::applicationDirPath() + QStringLiteral("/../share/icons/hicolor");
        const QStringList sizes = { QStringLiteral("256x256"), QStringLiteral("128x128"), QStringLiteral("64x64"),
                                    QStringLiteral("48x48"), QStringLiteral("32x32") };
        for (const QString &size : sizes) {
            const QString path = base + QChar('/') + size + QStringLiteral("/apps/freecad.png");
            if (QFile::exists(path)) {
                icon = QIcon(path);
                break;
            }
        }
    }
    if (icon.isNull() && QIcon::hasThemeIcon(QStringLiteral("freecad"))) {
        icon = QIcon::fromTheme(QStringLiteral("freecad"));
    }
    if (icon.isNull()) {
        icon = QIcon(QStringLiteral(":/freecad-icon.svg"));
    }
    return icon;
}
}

#endif
