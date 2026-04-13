#include "main_window.h"

#include <qdir.h>
#include <qlineedit.h>
#include <qthread.h>
#include <qtabbar.h>
#include <qnetworkinterface.h>
#include <qmessagebox.h>

#include <utils/debug_output.h>
#include <utils/file_io.h>

#include "config.h"
#include "search_ndi_sources_dialog.h"
#include "settings.h"

/// @brief 根据给定参数构建一个当前不存在的文件路径。
static QString makeAvailablePath(const QDir& dir, const QString& filename, const QString& fileext)
{
    int i = 0;
    while (true)
    {
        // 在文件名后加上序号。
        QString filepath = dir.filePath(filename + (i == 0 ? "" : QString(" (%1)").arg(i)) + fileext);
        if (!QFileInfo::exists(filepath))
            return filepath;
        ++i;
    }
}

MainWindow::MainWindow(QWidget* parent)
    : TrMainWindow(parent), tabWidgetAddBtn_(new QPushButton(this)),
    server_("LineCheatingServer", QWebSocketServer::NonSecureMode, this)
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
    try
    {
        // 读取 Work Config 列表。
        auto jsonContent = readFileContent(DEFAULT_WORK_CONFIG_FILEPATH);
        nlohmann::json j = nlohmann::json::parse(jsonContent.toStdString(), nullptr, true, true);

        if (j.contains("workConfigs") && j["workConfigs"].is_array())
        {
            auto workConfigsArr = j["workConfigs"];
            for (size_t i = 0; i < workConfigsArr.size(); ++i)
            {
                const auto& workConfigObj = workConfigsArr[i];
                WorkConfig config = WorkConfig::fromJson(workConfigObj);
                addTabPage(config, false);
            }
        }
    }
    catch (...)
    {
        // Pass
        ;
    }

    // 根据 TabWidget 的页面数量设置当前窗口（如果数量为 0 则显示启动界面）。
    ui.stackedWidget->setCurrentWidget(ui.tabWidget->count() == 0 ? ui.introPage : ui.tabWidgetPage);

    ui.introWidget->installEventFilter(this);
    ui.tabWidget->tabBar()->installEventFilter(this);

    // Action 信号槽
    connect(ui.actionSearchNdiSources, &QAction::triggered, this, &MainWindow::onSearchNdiActivated);

    // 配置 Web Socket 服务器
    Settings settings = loadSettings();
    if (!server_.listen(QHostAddress::Any, settings.serverPort))
        debugOut(qCritical(), "Failed to listen.");

    connect(&server_, &QWebSocketServer::newConnection, this, &MainWindow::onNewConnection);

    updateText();
}

MainWindow::~MainWindow()
{
    // 退出程序前保存所有工作页面的 Work Config。
    nlohmann::json workConfigsArr = nlohmann::json::array();

    // 尝试创建配置文件目录。
    QDir configDir(DEFAULT_ASSIST_PROGRAM_WORK_CONFIG_DIR);
    if (!configDir.exists())
    {
        if (!configDir.mkpath("."))
            debugOut(qCritical(), "Failed to create the directory: '%1'.", DEFAULT_ASSIST_PROGRAM_WORK_CONFIG_DIR);
    }

    // 获取每个工作页面的 Assist Program Work Config 并保存至指定/默认路径。
    for (int i = 0; i < ui.tabWidget->count(); ++i)
    {
        auto page = qobject_cast<AssistProgramOperatePage*>(ui.tabWidget->widget(i));
        if (!page)
            continue;

        WorkConfig config = pageAndConfigMap_[page];
        if (config.configPath.isEmpty())
            config.configPath = makeAvailablePath(QDir(DEFAULT_ASSIST_PROGRAM_WORK_CONFIG_DIR), "config", ".json");

        // 保存 Assist Program Work Config。
        try
        {
            page->getAssistProgramWorkConfig().toFile(config.configPath);
        }
        catch (const std::exception& e)
        {
            debugOut(
                qCritical(),
                "Can't write assist program work config to: '%1'. Error: %2.",
                config.configPath,
                e.what());
        }

        workConfigsArr.push_back(config.toJson());
    }

    nlohmann::json j;
    j["workConfigs"] = workConfigsArr;

    // 保存 Work Config 列表。
    try
    {
        writeFileContent(DEFAULT_WORK_CONFIG_FILEPATH, QString::fromStdString(j.dump(4)));
    }
    catch (const std::exception& e)
    {
        debugOut(
            qCritical(),
            "Can't write work config file: '%1'. Error: %2.",
            DEFAULT_GAME_DATA_FILEPATH,
            e.what());
    }

    // 退出并清理工作线程
    for (auto it = pageAndConfigMap_.begin(); it != pageAndConfigMap_.end(); ++it)
        cleanupWorkPage(it.key());

    // 关闭客户端连接
    for (auto socket : clients_)
    {
        socket->disconnect();
        socket->close();
        socket->deleteLater();
    }
    clients_.clear();

    // 关闭服务器
    server_.close();
}

void MainWindow::addTabPage(const WorkConfig& config, bool jumpTo)
{
    // 读取 Game Data （必需）。
    GameData gameData;
    try
    {
        gameData = GameData::fromFile(config.gameDataPath.toStdString());
    }
    catch (const std::exception& e)
    {
        debugOut(
            qWarning(),
            "Failed to open/parse Game Data from file: '%1'. Error: %2.",
            config.gameDataPath,
            e.what());
        return;
    }

    // 读取 Assist Program Work Config，如果读取失败则保持默认参数。
    AssistProgramWorkConfig assistProgramWorkConfig;
    try
    {
        assistProgramWorkConfig = AssistProgramWorkConfig::fromFile(config.configPath);
    }
    catch (const std::exception& e)
    {
        debugOut(
            qInfo(),
            "Failed to open/parse Assist Program Work Config from file: '%1'. Error: %2. Will create a new config.",
            config.configPath,
            e.what());
    }

    // 新建工作页面。
    auto page = new AssistProgramOperatePage(gameData, assistProgramWorkConfig);

    int index = ui.tabWidget->addTab(page, config.name);
    // 处理页面切换逻辑。
    if (ui.stackedWidget->currentWidget() != ui.tabWidgetPage)
        ui.stackedWidget->setCurrentWidget(ui.tabWidgetPage);
    if (jumpTo)
        ui.tabWidget->setCurrentIndex(index);

    pageAndConfigMap_.insert(page, config);
}

void MainWindow::addTabPage(bool jumpTo)
{
    // 当前默认创建 Assist Program。
    WorkConfig config;
    config.type = WT_ASSIST;
    config.name = EASYTR("New Work");
    config.gameDataPath = DEFAULT_GAME_DATA_FILEPATH;

    addTabPage(config, jumpTo);
}

void MainWindow::removeTabPage(int index)
{
    auto page = qobject_cast<AssistProgramOperatePage*>(ui.tabWidget->widget(index));
    if (!page)
        return;

    if (page->isRunning())
    {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle(EASYTR("Warning"));
        msgBox.setText(EASYTR("The work is running, are you sure exit the work and close page?"));

        auto confirmBtn = msgBox.addButton(EASYTR("Ok"), QMessageBox::AcceptRole);
        auto cancelBtn = msgBox.addButton(EASYTR("Cancel"), QMessageBox::RejectRole);
        msgBox.setDefaultButton(cancelBtn);

        msgBox.exec();

        if (msgBox.clickedButton() == cancelBtn)
            return;
    }

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
            // 启动页面双击创建新工作。
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
            // 标签栏重命名。
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

void MainWindow::onNewConnection()
{
    while (server_.hasPendingConnections())
    {
        auto socket = server_.nextPendingConnection();

        clients_.insert(socket);
        debugOut(qInfo(), "New client connected: %1.", socket->peerAddress().toString());

        connect(socket, &QWebSocket::textMessageReceived, this, &MainWindow::onTextMessageReceived);
        connect(socket, &QWebSocket::disconnected, this, &MainWindow::onClientDisconnected);
    }
}

void MainWindow::onClientDisconnected()
{
    auto socket = qobject_cast<QWebSocket*>(sender());
    if (!socket)
        return;

    debugOut(qInfo(), "Client disconnected: %1.", socket->peerAddress().toString());
    clients_.remove(socket);
    socket->deleteLater();
}

void MainWindow::onTextMessageReceived(const QString& msg)
{
    auto socket = qobject_cast<QWebSocket*>(sender());
    if (!socket)
        return;

    debugOut(qInfo(), "Received text message from: %1.", socket->peerAddress().toString());

    nlohmann::json j = nlohmann::json::parse(msg.toStdString(), nullptr,  false, true);
    if (j.is_discarded())
    {
        debugOut(qCritical(), "Failed parse json from client: %1.", socket->peerAddress().toString());
        return;
    }

    int index, flag;
    try
    {
        index = j.at("index");
        flag = j.at("flag");    // 1: run, 2: stop.
    }
    catch (const std::exception& e)
    {
        debugOut(qCritical(), "Illegal json data from client: %1.", socket->peerAddress().toString());
        return;
    }

    auto page = qobject_cast<AssistProgramOperatePage*>(ui.tabWidget->widget(index));
    if (!page)
    {
        debugOut(qWarning(), "Got index (%1) is invalid, from client: %2.", index, socket->peerAddress().toString());
        return;
    }

    switch (flag)
    {
        case 1:     page->run(); break;
        case 2:     page->stop(); break;
        default:    break;
    }
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
    if (!page)
        return;

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

void MainWindow::disconnectClient(QWebSocket* socket)
{
    if (!socket || !clients_.contains(socket))
        return;
    socket->close(QWebSocketProtocol::CloseCodeNormal, "Server closed connection");
}
