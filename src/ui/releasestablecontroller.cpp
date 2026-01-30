#include "releasestablecontroller.h"
#include "../interfaces/idownloader.h"
#include "../utils/filesizeformatter.h"
#include <QCoreApplication>
#include <QDateTime>
#include <QBrush>
#include <QColor>

ReleasesTableController::ReleasesTableController(QTableWidget *table, IDownloader *downloader)
    : m_table(table)
    , m_downloader(downloader)
{
}

QString ReleasesTableController::formatDate(const QString &isoDate)
{
    QDateTime dateTime = QDateTime::fromString(isoDate, Qt::ISODate);
    return dateTime.toString("yyyy-MM-dd");
}

void ReleasesTableController::setReleaseStatusCell(int row, const ReleaseDownloadStatus &status)
{
    QTableWidgetItem *statusItem = m_table->item(row, 4);
    if (!statusItem) {
        statusItem = new QTableWidgetItem();
        statusItem->setTextAlignment(Qt::AlignCenter);
        m_table->setItem(row, 4, statusItem);
    }
    statusItem->setText(status.isDownloaded ? "✓" : "");
    if (status.isDownloaded) {
        statusItem->setForeground(QBrush(QColor(0, 150, 0)));
        QString tooltip = QCoreApplication::translate("ReleasesTableController", "File already downloaded to working directory");
        if (!status.architecture.isEmpty()) {
            tooltip += QString(" (%1)").arg(status.architecture);
        }
        statusItem->setToolTip(tooltip);
    } else {
        statusItem->setForeground(QBrush());
        statusItem->setToolTip(QCoreApplication::translate("ReleasesTableController", "Not downloaded"));
    }
}

void ReleasesTableController::updateTable(const QList<GitHubRelease> &releases)
{
    m_table->setRowCount(releases.size());
    for (int i = 0; i < releases.size(); ++i) {
        const GitHubRelease &release = releases[i];
        m_table->setItem(i, 0, new QTableWidgetItem(release.tagName));
        m_table->setItem(i, 1, new QTableWidgetItem(release.name));
        m_table->setItem(i, 2, new QTableWidgetItem(formatDate(release.publishedAt)));
        qint64 size = 0;
        if (!release.appImages.isEmpty()) {
            for (const AppImageAsset &a : release.appImages) {
                if (a.architecture == QStringLiteral("x86_64")) {
                    size = a.fileSize;
                    break;
                }
            }
            if (size == 0) {
                size = release.appImages.first().fileSize;
            }
        }
        m_table->setItem(i, 3, new QTableWidgetItem(FileSizeFormatter::format(size)));
        setReleaseStatusCell(i, m_downloader->releaseDownloadStatus(release));
    }
    m_table->resizeColumnsToContents();
}

void ReleasesTableController::updateDownloadStatus(const QList<GitHubRelease> &releases)
{
    if (m_table->rowCount() != releases.size()) {
        return;
    }
    for (int i = 0; i < releases.size(); ++i) {
        setReleaseStatusCell(i, m_downloader->releaseDownloadStatus(releases[i]));
    }
}
