#include "main_window.h"

#include <qlineedit.h>
#include <qthread.h>
#include <qtabbar.h>

#include <utils/debug_output.h>

#include "config.h"
#include "search_ndi_sources_dialog.h"

MainWindow::MainWindow(QWidget* parent)
    : TrMainWindow(parent), tabWidgetAddBtn_(new QPushButton(this))
{
    ui.setupUi(this);

    // 初始化 TabWidget 的标签页增加按钮。
    tabWidgetAddBtn_->setIcon(QIcon(":/icons/add.png"));
    tabWidgetAddBtn_->setFlat(true);
    tabWidgetAddBtn_->setIconSize(QSize(24, 24));
    ui.tabWidget->setCornerWidget(tabWidgetAddBtn_);
    connect(tabWidgetAddBtn_, &QPushButton::clicked, this, [this]() { addTabPage(true); });

    // 标签页的关闭
    ui.tabWidget->setTabsClosable(true);
    connect(ui.tabWidget, &QTabWidget::tabCloseRequested, this, &MainWindow::onTabCloseRequested);

    // 根据应用设置读取已有页面。
    // TODO: Assign ui.tabWidget and configAndPageMap_.

    // 根据 TabWidget 的页面数量设置当前窗口（如果数量为 0 则显示启动界面）。
    ui.stackedWidget->setCurrentWidget(ui.tabWidget->count() == 0 ? ui.introPage : ui.tabWidgetPage);

    ui.introWidget->installEventFilter(this);
    ui.tabWidget->tabBar()->installEventFilter(this);

    // Action 信号槽
    connect(ui.actionSearchNdiSources, &QAction::triggered, this, &MainWindow::onSearchNdiActivated);

    updateText();
}

MainWindow::~MainWindow()
{
    // TODO: Save settings.

    // Cleanup work thread
    for (auto it = pageAndConfigMap_.begin(); it != pageAndConfigMap_.end(); ++it)
        cleanupWorkPage(it.key());
}

void MainWindow::addTabPage(bool jumpTo)
{
    // Changed in future.

    WorkConfig config;
    config.type = WT_ASSIST;
    config.name = EASYTR("New Work");
    config.gameDataPath = DEFAULT_GAME_DATA_FILEPATH;

    GameData gameData;
    try { gameData = GameData::fromFile(config.gameDataPath.toStdString()); }
    catch (const std::exception& e)
    {
        debugOut(qWarning(), "Failed to open/parse Game Data: %1", e.what());
        return;
    }

    AssistProgramWorkConfig assistProgramWorkConfig;
    assistProgramWorkConfig.footmanHidInfo = {1, 1};

    auto page = new AssistProgramOperatePage(gameData, assistProgramWorkConfig);

    int index = ui.tabWidget->addTab(page, config.name);
    if (ui.stackedWidget->currentWidget() != ui.tabWidgetPage)
        ui.stackedWidget->setCurrentWidget(ui.tabWidgetPage);
    if (jumpTo)
        ui.tabWidget->setCurrentIndex(index);

    pageAndConfigMap_.insert(page, config);
}

void MainWindow::removeTabPage(int index)
{
    auto page = ui.tabWidget->widget(index);

    // 先从界面中移除标签页。
    ui.tabWidget->removeTab(index);
    if (ui.tabWidget->count() == 0)
        ui.stackedWidget->setCurrentWidget(ui.introPage);

    // 删除键值对
    pageAndConfigMap_.remove(page);

    // 退出并删除工作线程及其页面。
    cleanupWorkPage(page);
}

void MainWindow::updateText()
{
    setWindowTitle(EASYTR("Lineage Cheating Tool"));

    // Tools menu
    ui.menuTools->setTitle(EASYTR("Tools"));
    ui.actionSearchNdiSources->setText(EASYTR("Search NDI Sources"));
    ui.actionSelectImagePoint->setText(EASYTR("Select Image Point"));
    ui.actionSelectScreenColor->setText(EASYTR("Select Screen Color"));
    ui.actionTestHid->setText(EASYTR("Test HID"));
    ui.actionSettings->setText(EASYTR("Settings"));

    // Help menu
    ui.menuHelp->setTitle(EASYTR("Help"));
    ui.actionAbout->setText(EASYTR("About"));

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
                addTabPage(true);
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
                return true;
            }
            default:
                break;
        }
    }

    return TrMainWindow::eventFilter(obj, event);
}

void MainWindow::onTabCloseRequested(int index)
{
    removeTabPage(index);
}

void MainWindow::onSearchNdiActivated()
{
    SearchNdiSourcesDialog dlg;
    QVariant source = dlg.getSelectedNdiSource();
}

void MainWindow::cleanupWorkPage(QWidget* wgt)
{
    auto page = qobject_cast<AssistProgramOperatePage*>(wgt);
    if (!page)
        return;

    // 执行标签页所属工作的退出工作。
    QThread* cleanupThread = QThread::create([page]() { page->stop(); });
    connect(cleanupThread, &QThread::finished, cleanupThread, [page, cleanupThread]
    {
        page->deleteLater();
        cleanupThread->deleteLater();
    });

    cleanupThread->start();
}

void MainWindow::startRenameTab(int index)
{
    auto page = qobject_cast<AssistProgramOperatePage*>(ui.tabWidget->widget(index));

    QTabBar* tabBar = ui.tabWidget->tabBar();
    QRect rect = tabBar->tabRect(index);

    auto editor = new QLineEdit(tabBar);
    editor->setText(ui.tabWidget->tabText(index));
    editor->setGeometry(rect);
    editor->selectAll();
    editor->setFocus();
    editor->show();

    connect(editor, &QLineEdit::editingFinished, this, [this, editor, page]()
    {
        QString newName = editor->text().trimmed();
        if (!newName.isEmpty())
        {
            int index = ui.tabWidget->indexOf(page);
            if (index != -1)
                ui.tabWidget->setTabText(index, newName);

            if (page)
            {
                WorkConfig config = pageAndConfigMap_[page];
                config.name = newName;
                pageAndConfigMap_[page] = config;
            }
        }
        editor->deleteLater();
    });
}
