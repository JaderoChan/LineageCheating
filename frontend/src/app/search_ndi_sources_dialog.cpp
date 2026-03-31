#include "search_ndi_sources_dialog.h"

#include <qmessagebox.h>

SearchNdiSourcesDialog::SearchNdiSourcesDialog(QWidget* parent)
    : TrDialog(parent), loadingMovie_(":/icons/loading.gif")
{
    ui.setupUi(this);

    ui.loadingIcon->setMovie(&loadingMovie_);
    ui.loadingIcon->hide();
    ui.tipTextLabel->setText(QString::fromStdString(EASYTR("Find %1 NDI sources.")).arg(0));

    connect(ui.refreshButton, &QPushButton::clicked, this, &SearchNdiSourcesDialog::onRefreshButtonClicked);
    connect(ui.confirmButtton, &QPushButton::clicked, this, &SearchNdiSourcesDialog::onConfirmButtonClicked);
    connect(ui.cancelButton, &QPushButton::clicked, this, &SearchNdiSourcesDialog::onCancelButtonClicked);

    refresh();

    updateText();
}

SearchNdiSourcesDialog::~SearchNdiSourcesDialog()
{
    workerThread_.quit();
    workerThread_.wait();
}

QVariant SearchNdiSourcesDialog::getSelectedNdiSource()
{
    if (exec() == QDialog::Rejected || selectedSourceName_.isEmpty())
        return QVariant();
    else
        return selectedSourceName_;
}

void SearchNdiSourcesDialog::updateText()
{
    setWindowTitle(EASYTR("Search NDI Sources"));

    ui.refreshButton->setText(EASYTR("Refresh"));
    ui.confirmButtton->setText(EASYTR("Confirm"));
    ui.cancelButton->setText(EASYTR("Cancel"));
}

void SearchNdiSourcesDialog::onRefreshButtonClicked()
{
    refresh();
}

void SearchNdiSourcesDialog::onConfirmButtonClicked()
{
    auto selecteds = ui.listWidget->selectedItems();
    if (!selecteds.empty())
        selectedSourceName_ = selecteds.front()->data(Qt::UserRole).toString();
    done(QDialog::Accepted);
}

void SearchNdiSourcesDialog::onCancelButtonClicked()
{
    done(QDialog::Rejected);
}

void SearchNdiSourcesDialog::refresh()
{
    if (workerThread_.isRunning())
        return;

    // 清空原先列表
    ui.listWidget->clear();
    selectedSourceName_.clear();

    // 禁用刷新按钮
    ui.refreshButton->setEnabled(false);

    // 显示加载动画
    ui.tipTextLabel->hide();
    ui.loadingIcon->show();
    loadingMovie_.start();

    // 查找 NDI 源
    auto worker = new NdiFindWorker();
    worker->moveToThread(&workerThread_);

    connect(&workerThread_, &QThread::started, worker, &NdiFindWorker::doFind);
    connect(worker, &NdiFindWorker::finished, this, &SearchNdiSourcesDialog::onFindFinished);
    connect(worker, &NdiFindWorker::finished, worker, &QObject::deleteLater);
    connect(worker, &NdiFindWorker::finished, &workerThread_, &QThread::quit);

    workerThread_.start();
}

void SearchNdiSourcesDialog::onFindFinished(const QList<QPair<QString, QString>>& sources, bool success)
{
    if (!success)
    {
        QMessageBox::warning(this, EASYTR("Warning"), EASYTR("Failed to create NDI finder."));
    }
    else
    {
        for (const auto& [name, url] : sources)
        {
            QString itemText = QString("%1 (%2)").arg(name, url);
            auto item = new QListWidgetItem(itemText, ui.listWidget);
            item->setData(Qt::UserRole, name);
        }
    }

    // 更改提示文本
    ui.tipTextLabel->setText(QString::fromStdString(EASYTR("Find %1 NDI sources.")).arg(ui.listWidget->count()));

    // 隐藏加载动画并显示提示文本
    loadingMovie_.stop();
    ui.loadingIcon->hide();
    ui.tipTextLabel->show();
    ui.refreshButton->setEnabled(true);

    // 启用刷新按钮
    ui.refreshButton->setEnabled(true);
}
