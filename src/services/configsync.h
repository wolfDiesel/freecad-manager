#ifndef CONFIGSYNC_H
#define CONFIGSYNC_H

#include <QString>
#include "../interfaces/iconfigsync.h"

class ConfigSync : public IConfigSync
{
    Q_OBJECT
public:
    explicit ConfigSync(QObject *parent = nullptr);

    bool syncFromHome(const QString &portableConfigDir, const QString &portableDataDir) override;
    bool syncToHome(const QString &portableConfigDir, const QString &portableDataDir) override;

    static QString getHomeConfigDir();
    static QString getHomeDataDir();

private:
    bool copyDirectory(const QString &source, const QString &destination, bool overwrite = false);
    bool copyFile(const QString &source, const QString &destination, bool overwrite = false);
};

#endif
