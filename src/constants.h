#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QByteArray>
#include <QString>

namespace GitHub {
const QString ApiReleasesUrl = QStringLiteral("https://api.github.com/repos/FreeCAD/FreeCAD/releases");
const int ReleasesPerPage = 100;
}

namespace App {
const QByteArray UserAgent = QByteArrayLiteral("FreeCAD-Launcher/1.0");
}

namespace PortableConfig {
inline QString fcadDir(const QString &baseDir) {
    return baseDir + QStringLiteral("/fcad");
}
inline QString configDir(const QString &baseDir) {
    return fcadDir(baseDir) + QStringLiteral("/.config/FreeCAD");
}
inline QString dataDir(const QString &baseDir) {
    return fcadDir(baseDir) + QStringLiteral("/.local/share/FreeCAD");
}
inline QString xdgConfigHome(const QString &baseDir) {
    return fcadDir(baseDir) + QStringLiteral("/.config");
}
inline QString xdgDataHome(const QString &baseDir) {
    return fcadDir(baseDir) + QStringLiteral("/.local/share");
}
}

#endif
