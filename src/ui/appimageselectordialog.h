#ifndef APPIMAGESELECTORDIALOG_H
#define APPIMAGESELECTORDIALOG_H

#include <QDialog>
#include "../types.h"

class QListWidget;

class AppImageSelectorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AppImageSelectorDialog(const GitHubRelease &release, QWidget *parent = nullptr);
    AppImageAsset selectedAppImage() const;

private:
    QListWidget *m_listWidget;
    QList<AppImageAsset> m_appImages;
};

#endif
