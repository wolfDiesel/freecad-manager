#ifndef ICONFIGSYNC_H
#define ICONFIGSYNC_H

#include <QObject>
#include <QString>

class IConfigSync : public QObject
{
    Q_OBJECT
public:
    explicit IConfigSync(QObject *parent = nullptr) : QObject(parent) {}

    virtual bool syncFromHome(const QString &portableConfigDir, const QString &portableDataDir) = 0;
    virtual bool syncToHome(const QString &portableConfigDir, const QString &portableDataDir) = 0;

signals:
    void syncProgress(const QString &message);
    void syncFinished(bool success, const QString &message);
    void syncError(const QString &error);
};

#endif
