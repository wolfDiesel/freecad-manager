#include "appimageselectordialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QListWidgetItem>
#include <QDialogButtonBox>
#include <QLabel>
#include "../utils/filesizeformatter.h"

AppImageSelectorDialog::AppImageSelectorDialog(const GitHubRelease &release, QWidget *parent)
    : QDialog(parent)
    , m_listWidget(nullptr)
    , m_appImages(release.appImages)
{
    setWindowTitle(tr("Select architecture"));
    setMinimumWidth(500);
    
    QVBoxLayout *layout = new QVBoxLayout(this);
    
    QLabel *label = new QLabel(tr("Several AppImage variants available. Select one:"), this);
    layout->addWidget(label);
    
    m_listWidget = new QListWidget(this);
    int defaultIndex = -1;
    
    for (int i = 0; i < m_appImages.size(); ++i) {
        const AppImageAsset &img = m_appImages[i];
        QString sizeStr = FileSizeFormatter::format(img.fileSize);
        
        QString displayText = QString("%1 (%2, %3)")
            .arg(img.fileName)
            .arg(img.architecture)
            .arg(sizeStr);
        
        QListWidgetItem *item = new QListWidgetItem(displayText, m_listWidget);
        item->setData(Qt::UserRole, i);
        
        if (img.architecture == "x86_64" && defaultIndex == -1) {
            defaultIndex = i;
        }
    }
    
    if (defaultIndex >= 0) {
        m_listWidget->setCurrentRow(defaultIndex);
    } else {
        m_listWidget->setCurrentRow(0);
    }
    
    layout->addWidget(m_listWidget);
    
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttonBox);
}

AppImageAsset AppImageSelectorDialog::selectedAppImage() const
{
    if (m_appImages.isEmpty()) {
        AppImageAsset empty;
        return empty;
    }
    
    if (m_appImages.size() == 1) {
        return m_appImages.first();
    }
    
    if (result() == QDialog::Accepted) {
        QListWidgetItem *currentItem = m_listWidget->currentItem();
        if (currentItem) {
            int index = currentItem->data(Qt::UserRole).toInt();
            if (index >= 0 && index < m_appImages.size()) {
                return m_appImages[index];
            }
        }
    }
    AppImageAsset empty;
    return empty;
}
