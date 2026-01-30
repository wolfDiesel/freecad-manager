#ifndef RELEASESTABLECONTROLLER_H
#define RELEASESTABLECONTROLLER_H

#include <QTableWidget>
#include <QList>
#include "../types.h"
#include "../interfaces/idownloader.h"

class ReleasesTableController
{
public:
    explicit ReleasesTableController(QTableWidget *table, IDownloader *downloader);

    void updateTable(const QList<GitHubRelease> &releases);
    void updateDownloadStatus(const QList<GitHubRelease> &releases);

private:
    void setReleaseStatusCell(int row, const ReleaseDownloadStatus &status);
    static QString formatDate(const QString &isoDate);

    QTableWidget *m_table;
    IDownloader *m_downloader;
};

#endif
