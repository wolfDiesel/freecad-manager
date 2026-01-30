#ifndef ITRAYMANAGER_H
#define ITRAYMANAGER_H

#include <QObject>
#include <QList>
#include "../types.h"

class QWidget;

class ITrayManager : public QObject
{
    Q_OBJECT
public:
    explicit ITrayManager(QObject *parent = nullptr) : QObject(parent) {}

    virtual void setup(QWidget *mainWindow) = 0;
    virtual void updateVersionsMenu(const QList<AppImageInfo> &appImages) = 0;
    virtual bool isAvailable() const = 0;
    virtual bool isVisible() const = 0;

signals:
    void showWindowRequested();
    void quitRequested();
    void launchVersionRequested(const AppImageInfo &appImage);
};

#endif
