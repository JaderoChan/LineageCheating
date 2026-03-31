#include "main_window.h"

#include <qthread.h>

#include "work_config.h"

MainWindow::MainWindow(QWidget* parent)
    : TrMainWindow(parent), tabWidgetAddBtn_(new QPushButton(this))
{
    ui.setupUi(this);

    // 初始化 `TabWidget` 的标签页增加按钮。
    tabWidgetAddBtn_->setIcon(QIcon(":/icons/add.png"));
    tabWidgetAddBtn_->setFlat(true);
    tabWidgetAddBtn_->setIconSize(QSize(24, 24));
    ui.tabWidget->setCornerWidget(tabWidgetAddBtn_);

    // 标签页的关闭
    ui.tabWidget->setTabsClosable(true);
    connect(ui.tabWidget, QTabWidget::tabCloseRequested, this, &MainWindow::onTabCloseRequested);

    // 根据应用设置读取已有页面。
    // TODO: Assign `ui.tabWidget`.

    // 根据 `TabWidget` 的页面数量设置当前窗口（如果数量为 `0` 则显示启动界面）。
    ui.stackedWidget->setCurrentWidget(ui.tabWidget->count() == 0 ? ui.introPage : ui.tabWidgetPage);

    ui.introWidget->installEventFilter(this);

    updateText();
}

void MainWindow::addWorkOperatePage(WorkOperatePage* page, bool jumpTo)
{
    int index = ui.tabWidget->addTab(page, page->getWorkConfig().name);
    if (ui.stackedWidget->currentWidget() != ui.tabWidgetPage)
        ui.stackedWidget->setCurrentWidget(ui.tabWidgetPage);
    if (jumpTo)
        ui.tabWidget->setCurrentIndex(index);
}

void MainWindow::removeWorkOperatePage(int index)
{
    auto page = qobject_cast<WorkOperatePage*>(ui.tabWidget->widget(index));
    if (!page)
        return;

    // 先从界面中移除标签页。
    ui.tabWidget->removeTab(index);
    if (ui.tabWidget->count() == 0)
        ui.stackedWidget->setCurrentWidget(ui.introPage);

    // 执行标签页所属工作的退出工作。
    QThread* cleanupThread = QThread::create([page]() { page->stop(); });
    connect(cleanupThread, &QThread::finished, this, [page, cleanupThread]
    {
        page->deleteLater();
        cleanupThread->deleteLater();
    });

    cleanupThread->start();
}

void MainWindow::removeWorkOperatePage(WorkOperatePage* page)
{
    int index = ui.tabWidget->indexOf(page);
    if (index != -1)
        removeWorkOperatePage(index);
}

void MainWindow::updateText()
{
    // Actions
    ui.actionSearchHIDSources->setText(EASYTR("Search HID Sources"));
    ui.actionSelectImagePoint->setText(EASYTR("Select Image Point"));
    ui.actionSelectScreenColor->setText(EASYTR("Select Screen Color"));
    ui.actionTestHID->setText(EASYTR("Test HID"));

    ui.introTextLabel->setText(EASYTR("Current no work be set, please double click page to add a new work."));
}

bool MainWindow::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == ui.introWidget)
    {
        switch (event->type())
        {
            case QEvent::MouseButtonDblClick:
            {
                WorkConfig config;
                config.name = EASYTR("New work");
                auto page = new WorkOperatePage(config, this);
                addWorkOperatePage(page, true);
                return true;
            }
            default:
                break;
        }
    }

    return false;
}

void MainWindow::onTabCloseRequested(int index)
{
    removeWorkOperatePage(index);
}
