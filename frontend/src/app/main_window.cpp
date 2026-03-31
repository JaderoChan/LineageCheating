#include "main_window.h"

#include <qlineedit.h>
#include <qthread.h>
#include <qtabbar.h>

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
    connect(tabWidgetAddBtn_, &QPushButton::clicked, this, [this]() { addWorkOperatePage(true); });

    // 标签页的关闭
    ui.tabWidget->setTabsClosable(true);
    connect(ui.tabWidget, &QTabWidget::tabCloseRequested, this, &MainWindow::onTabCloseRequested);

    // 根据应用设置读取已有页面。
    // TODO: Assign `ui.tabWidget`.

    // 根据 `TabWidget` 的页面数量设置当前窗口（如果数量为 `0` 则显示启动界面）。
    ui.stackedWidget->setCurrentWidget(ui.tabWidget->count() == 0 ? ui.introPage : ui.tabWidgetPage);

    ui.introWidget->installEventFilter(this);
    ui.tabWidget->tabBar()->installEventFilter(this);

    updateText();
}

MainWindow::~MainWindow()
{
    // TODO: Save settings.
}

void MainWindow::addWorkOperatePage(WorkOperatePage* page, bool jumpTo)
{
    int index = ui.tabWidget->addTab(page, page->getWorkConfig().name);
    if (ui.stackedWidget->currentWidget() != ui.tabWidgetPage)
        ui.stackedWidget->setCurrentWidget(ui.tabWidgetPage);
    if (jumpTo)
        ui.tabWidget->setCurrentIndex(index);
}

void MainWindow::addWorkOperatePage(bool jumpTo)
{
    WorkConfig config;
    config.name = EASYTR("New Work");
    auto page = new WorkOperatePage(config);
    addWorkOperatePage(page, jumpTo);
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
    connect(cleanupThread, &QThread::finished, cleanupThread, [page, cleanupThread]
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
    ui.actionSearchNdiSources->setText(EASYTR("Search NDI Sources"));
    ui.actionSelectImagePoint->setText(EASYTR("Select Image Point"));
    ui.actionSelectScreenColor->setText(EASYTR("Select Screen Color"));
    ui.actionTestHid->setText(EASYTR("Test HID"));

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
                addWorkOperatePage(true);
                return true;
            }
            default:
                break;
        }
    }
    else if (obj == ui.tabWidget->tabBar())
    {
        switch (event->type())
        {
            case QEvent::MouseButtonDblClick:
            {
                auto mouseEvent = static_cast<QMouseEvent*>(event);
                int index = ui.tabWidget->tabBar()->tabAt(mouseEvent->pos());
                if (index != -1)
                    startRenameTab(index);
            }
            default:
                break;
        }
    }

    return TrMainWindow::eventFilter(obj, event);
}

void MainWindow::onTabCloseRequested(int index)
{
    removeWorkOperatePage(index);
}

void MainWindow::startRenameTab(int index)
{
    QTabBar* tabBar = ui.tabWidget->tabBar();
    QRect rect = tabBar->tabRect(index);

    auto editor = new QLineEdit(tabBar);
    editor->setText(ui.tabWidget->tabText(index));
    editor->setGeometry(rect);
    editor->selectAll();
    editor->setFocus();
    editor->show();

    connect(editor, &QLineEdit::editingFinished, this, [this, editor, index]()
    {
        QString newName = editor->text().trimmed();
        if (!newName.isEmpty())
        {
            ui.tabWidget->setTabText(index, newName);
            auto page = qobject_cast<WorkOperatePage*>(ui.tabWidget->widget(index));
            if (page)
            {
                WorkConfig config = page->getWorkConfig();
                config.name = newName;
                page->setWorkConfig(config);
            }
        }
        editor->deleteLater();
    });
}
