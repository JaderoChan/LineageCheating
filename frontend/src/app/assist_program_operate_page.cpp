#include "assist_program_operate_page.h"

#include <chrono>

#include <qvalidator.h>
#include <qmessagebox.h>

#include <utils/debug_output.h>

#include "edit_assist_program_config_dialog.h"
#include "search_ndi_sources_dialog.h"

static bool isValidHid(hid::HID hid)
{
    return hid && (reinterpret_cast<intptr_t>(hid) != -1);
}

bool isNdiConnected(NDIlib_recv_instance_t recv, uint32_t timeout)
{
    using namespace std::chrono;

    if (!recv)
        return false;

    NDIlib_video_frame_v2_t videoFrame;
    NDIlib_audio_frame_v3_t audioFrame;
    NDIlib_metadata_frame_t metadataFrame;

    milliseconds timeoutMs(timeout);
    auto start = high_resolution_clock::now();
    while (high_resolution_clock::now() - start < timeoutMs)
    {
        auto frameType = NDIlib_recv_capture_v3(recv, &videoFrame, &audioFrame, &metadataFrame, 50);
        switch (frameType)
        {
            case NDIlib_frame_type_video:
                NDIlib_recv_free_video_v2(recv, &videoFrame);
                return true;
            case NDIlib_frame_type_audio:
                NDIlib_recv_free_audio_v3(recv, &audioFrame);
                return true;
            case NDIlib_frame_type_metadata:
                NDIlib_recv_free_metadata(recv, &metadataFrame);
                return true;
            case NDIlib_frame_type_status_change:
                return true;
            default:
                break;
        }
    }

    return false;
}

AssistProgramOperatePage::AssistProgramOperatePage(
    const GameData& gameData, const AssistProgramWorkConfig& config, QWidget* parent)
    : TrWidget(parent), gameData_(gameData), config_(config),
    redCirclePixmap_(":/icons/red_circle.png"), greenCirclePixmap_(":/icons/green_circle.png"),
    alertPixmap_(":/icons/alert.png"), passPixmap_(":/icons/pass.png")
{
    ui.setupUi(this);

    // 初始化文本
    ui.masterNdiSourceNameInputLineEdit->setText(config_.masterNdiSourceName);
    ui.footmanNdiSourceNameInputLineEdit->setText(config_.footmanNdiSourceName);
    ui.footmanHidVidInputLineEdit->setText(QString::number(config_.footmanHidInfo.vid));
    ui.footmanHidPidInputLineEdit->setText(QString::number(config_.footmanHidInfo.pid));

    // 设置 VID 和 PID 的输入验证器，使其只接受正整数。
    auto vidValidator = new QIntValidator(1, 65535, this);
    auto pidValidator = new QIntValidator(1, 65535, this);
    ui.footmanHidVidInputLineEdit->setValidator(vidValidator);
    ui.footmanHidPidInputLineEdit->setValidator(pidValidator);

    // 设置运行状态检查频率。
    runningStateUpdateTimer_.setInterval(200);

    // 信号槽
    connect(ui.editConfigButton, &QPushButton::clicked, this, &AssistProgramOperatePage::onEditConfigButtonClicked);

    connect(ui.masterNdiSourceNameInputLineEdit, &QLineEdit::editingFinished,
        this, [this]() { config_.masterNdiSourceName = ui.masterNdiSourceNameInputLineEdit->text(); });
    connect(ui.footmanNdiSourceNameInputLineEdit, &QLineEdit::editingFinished,
        this, [this]() { config_.footmanNdiSourceName = ui.footmanHidPidInputLineEdit->text(); });
    connect(ui.footmanHidVidInputLineEdit, &QLineEdit::editingFinished,
        this, [this]() { config_.footmanHidInfo.vid = ui.footmanHidVidInputLineEdit->text().toUInt(); });
    connect(ui.footmanHidPidInputLineEdit, &QLineEdit::editingFinished,
        this, [this]() { config_.footmanHidInfo.pid = ui.footmanHidPidInputLineEdit->text().toUInt(); });

    connect(ui.masterSearchNdiSourceButton, &QPushButton::clicked,
        this, [this]() { onSearchNdiSourceButtonClicked(Master); });
    connect(ui.footmanSearchNdiSourceButton, &QPushButton::clicked,
        this, [this]() { onSearchNdiSourceButtonClicked(Footman); });
    connect(ui.masterNdiConnectButton, &QPushButton::clicked,
        this, [this]() { onNdiConnectButtonClicked(Master); });
    connect(ui.footmanNdiConnectButton, &QPushButton::clicked,
        this, [this]() { onNdiConnectButtonClicked(Footman); });
    connect(ui.footmanHidConnectButton, &QPushButton::clicked,
        this, &AssistProgramOperatePage::onHidConnectButtonClicked);

    connect(ui.startButton, &QPushButton::clicked, this, &AssistProgramOperatePage::run);
    connect(ui.stopButton, &QPushButton::clicked, this, &AssistProgramOperatePage::stop);

    connect(&runningStateUpdateTimer_, &QTimer::timeout, this, &AssistProgramOperatePage::updateRunningStateWidgets);

    runningStateUpdateTimer_.start();

    updateText();
    updateConnectStateWidgets();
}

AssistProgramOperatePage::~AssistProgramOperatePage()
{
    // 退出工作线程
    stop();

    // 销毁 NDI 接收器
    if (masterRecv_)
        NDIlib_recv_destroy(masterRecv_);
    if (footmanRecv_)
        NDIlib_recv_destroy(footmanRecv_);

    // 关闭 HID 设备
    if (isValidHid(hid_))
        hid::closeHID(hid_);
}

GameData AssistProgramOperatePage::getGameData() const
{
    return gameData_;
}

void AssistProgramOperatePage::setGameData(const GameData& gameData)
{
    gameData_ = gameData;
    if (assistProgram_)
        assistProgram_->setGameData(gameData);
}

AssistProgramWorkConfig AssistProgramOperatePage::getAssistProgramWorkConfig() const
{
    return config_;
}

void AssistProgramOperatePage::setAssistProgramWorkConfig(const AssistProgramWorkConfig& config)
{
    config_ = config;
    if (assistProgram_)
        assistProgram_->setConfig(config_.config);
}

void AssistProgramOperatePage::run()
{
    if (isRunning())
        return;

    if (!masterNdiConnected_ || !footmanNdiConnected_ || !footmanHidConnected_)
    {
        QMessageBox::warning(
            this,
            EASYTR("Warning"),
            EASYTR("Please configure master host and footman host first."),
            EASYTR("Ok"));
        return;
    }

    assistProgram_ = std::make_unique<AssistProgram>(masterRecv_, footmanRecv_, hid_, gameData_, config_.config);
    assistProgram_->run();

    updateRunningStateWidgets();
}

void AssistProgramOperatePage::stop()
{
    if (!isRunning())
        return;

    assistProgram_->stop();
    assistProgram_.reset();

    updateRunningStateWidgets();
}

bool AssistProgramOperatePage::isRunning() const
{
    if (assistProgram_)
        return assistProgram_->isRunning();
    return false;
}

void AssistProgramOperatePage::updateText()
{
    // Info Bar
    ui.titleLabel->setText(EASYTR("Assist Program"));
    ui.runningStateTextLabel->setText(EASYTR("(Not Running)"));
    ui.editConfigButton->setToolTip(EASYTR("Edit assist program config"));

    // Master Host Group Box
    ui.masterConfigureGroupBox->setTitle(EASYTR("Master Host"));
    ui.masterNdiTextLabel->setText(EASYTR("NDI Source"));
    ui.masterNdiSourceNameInputLineEdit->setPlaceholderText(EASYTR("Master NDI Source Name"));
    ui.masterSearchNdiSourceButton->setToolTip(EASYTR("Search and select NDI source"));
    ui.masterNdiConnectButton->setText(EASYTR("Connect NDI Source"));

    // Footman Host Group Box
    ui.footmanConfigureGroupBox->setTitle(EASYTR("Footman Host"));
    ui.footmanNdiTextLabel->setText(EASYTR("NDI Source"));
    ui.footmanNdiSourceNameInputLineEdit->setPlaceholderText(EASYTR("Footman NDI Source Name"));
    ui.footmanSearchNdiSourceButton->setToolTip(EASYTR("Search and select NDI source"));
    ui.footmanNdiConnectButton->setText(EASYTR("Connect NDI Source"));
    ui.footmanHidTextLabel->setText(EASYTR("HID"));
    ui.footmanHidVidTextLabel->setText(EASYTR("VID"));
    ui.footmanHidPidTextLabel->setText(EASYTR("PID"));
    ui.footmanHidConnectButton->setText(EASYTR("Connect HID Device"));

    // Operate
    ui.startButton->setText(EASYTR("Start Run"));
    ui.stopButton->setText(EASYTR("Stop Run"));
}

void AssistProgramOperatePage::onEditConfigButtonClicked()
{
    // 编辑 Assist Program Config（并非 Assist Program Work Config）。
    AssistProgramWorkConfig config = config_;
    EditAssistProgramConfigDialog dlg(config.config);

    bool isAccept = false;
    config.config = dlg.execForConfig(isAccept);
    if (isAccept)
        setAssistProgramWorkConfig(config);
}

void AssistProgramOperatePage::onNdiConnectButtonClicked(HostFlag flag)
{
    bool& connected = (flag == Master ? masterNdiConnected_ : footmanNdiConnected_);
    NDIlib_recv_instance_t& recv = (flag == Master ? masterRecv_ : footmanRecv_);

    if (connected)
    {
        // 断开逻辑
        if (isRunning())
        {
            QMessageBox msgBox(this);
            msgBox.setWindowTitle(EASYTR("Warning"));
            msgBox.setText(EASYTR("The work is running, are you sure exit the work and disconnect?"));

            auto confirmBtn = msgBox.addButton(EASYTR("Ok"), QMessageBox::AcceptRole);
            auto cancelBtn = msgBox.addButton(EASYTR("Cancel"), QMessageBox::RejectRole);
            msgBox.setDefaultButton(cancelBtn);

            msgBox.exec();

            if (msgBox.clickedButton() == cancelBtn)
                return;
            stop();
        }

        if (recv)
        {
            NDIlib_recv_destroy(recv);
            recv = nullptr;
        }

        connected = false;
    }
    else
    {
        // 连接逻辑
        NDIlib_recv_create_v3_t recvDesc = {0};
        recvDesc.color_format = NDIlib_recv_color_format_BGRX_BGRA;
        recv = NDIlib_recv_create_v3(&recvDesc);
        if (!recv)
        {
            QMessageBox::critical(
                this,
                EASYTR("Error"),
                EASYTR("Failed to create the NDI recevier."),
                EASYTR("Ok"));
            return;
        }

        QString sourceName = (flag == Master ? config_.masterNdiSourceName : config_.footmanNdiSourceName);
        QByteArray name = sourceName.toUtf8();

        NDIlib_source_t source = {0};
        source.p_ndi_name = name.constData();

        NDIlib_recv_connect(recv, &source);

        if (!isNdiConnected(recv, 500))
        {
            NDIlib_recv_destroy(recv);
            recv = nullptr;
            QMessageBox::critical(
                this,
                EASYTR("Error"),
                EASYTR("Failed to connect the NDI source."),
                EASYTR("Ok"));
            return;
        }

        connected = true;
    }

    updateConnectStateWidgets();
}

void AssistProgramOperatePage::onSearchNdiSourceButtonClicked(HostFlag flag)
{
    if (isRunning())
    {
        QMessageBox::information(
            this,
            EASYTR("Warning"),
            EASYTR("Please stop work first."),
            EASYTR("Ok"));
        return;
    }

    SearchNdiSourcesDialog dlg;
    QVariant source = dlg.getSelectedNdiSource();
    if (!source.isNull())
    {
        switch (flag)
        {
            case Master:
                // 如果处于连接状态，先断开连接。
                if (masterNdiConnected_)
                    onNdiConnectButtonClicked(flag);

                config_.masterNdiSourceName = source.toString();
                ui.masterNdiSourceNameInputLineEdit->setText(config_.masterNdiSourceName);
                break;
            case Footman:
                if (footmanNdiConnected_)
                    onNdiConnectButtonClicked(flag);

                config_.footmanNdiSourceName = source.toString();
                ui.footmanNdiSourceNameInputLineEdit->setText(config_.footmanNdiSourceName);
                break;
            default:
                break;
        }
    }
}

void AssistProgramOperatePage::onHidConnectButtonClicked()
{
    if (footmanHidConnected_)
    {
        // 断开逻辑
        if (isRunning())
        {
            QMessageBox msgBox(this);
            msgBox.setWindowTitle(EASYTR("Warning"));
            msgBox.setText(EASYTR("The work is running, are you sure exit the work and disconnect?"));

            auto confirmBtn = msgBox.addButton(EASYTR("Ok"), QMessageBox::AcceptRole);
            auto cancelBtn = msgBox.addButton(EASYTR("Cancel"), QMessageBox::RejectRole);
            msgBox.setDefaultButton(cancelBtn);

            msgBox.exec();

            if (msgBox.clickedButton() == cancelBtn)
                return;
            stop();
        }

        hid::closeHID(hid_);
        footmanHidConnected_ = false;
    }
    else
    {
        // 连接逻辑
        hid_ = hid::openHID(config_.footmanHidInfo.vid, config_.footmanHidInfo.pid);
        if (!isValidHid(hid_))
        {
            QMessageBox::critical(
                this,
                EASYTR("Error"),
                EASYTR("Can't connect HID device."),
                EASYTR("Ok"));
            hid_ = nullptr;
        }
        else
        {
            footmanHidConnected_ = true;
        }
    }

    updateConnectStateWidgets();
}

void AssistProgramOperatePage::updateRunningStateWidgets()
{
    static bool lastIsRunning = false;
    bool currentIsRunning = isRunning();
    // 运行状态发生了改变，更新信息。
    if (lastIsRunning != currentIsRunning)
    {
        if (currentIsRunning)
        {
            ui.runningStateIcon->setPixmap(greenCirclePixmap_);
            ui.runningStateTextLabel->setText(EASYTR("(Is Running)"));
        }
        else
        {
            ui.runningStateIcon->setPixmap(redCirclePixmap_);
            ui.runningStateTextLabel->setText(EASYTR("(Not Running)"));

            // 额外判断 NDI 是否断开。
            if (!isNdiConnected(masterRecv_, 200))
            {
                masterNdiConnected_ = false;
                if (masterRecv_)
                {
                    NDIlib_recv_destroy(masterRecv_);
                    masterRecv_ = nullptr;
                }
            }

            if (!isNdiConnected(footmanRecv_, 200))
            {
                footmanNdiConnected_ = false;
                if (footmanRecv_)
                {
                    NDIlib_recv_destroy(footmanRecv_);
                    footmanRecv_ = nullptr;
                }
            }

            updateConnectStateWidgets();
        }

        lastIsRunning = currentIsRunning;
    }
}

void AssistProgramOperatePage::updateConnectStateWidgets()
{
    // 设置连接状态图标。
    ui.masterNdiStateIcon->setPixmap(masterNdiConnected_ ? passPixmap_ : alertPixmap_);
    ui.footmanNdiStateIcon->setPixmap(footmanNdiConnected_ ? passPixmap_ : alertPixmap_);
    ui.footmanHidStateIcon->setPixmap(footmanHidConnected_ ? passPixmap_ : alertPixmap_);

    // 设置连接按钮文本。
    ui.masterNdiConnectButton->setText(
        masterNdiConnected_ ? EASYTR("Disconnect NDI Source") : EASYTR("Connect NDI Source"));
    ui.footmanNdiConnectButton->setText(
        footmanNdiConnected_ ? EASYTR("Disconnect NDI Source") : EASYTR("Connect NDI Source"));
    ui.footmanHidConnectButton->setText(
        footmanHidConnected_ ? EASYTR("Disconnect HID Device") : EASYTR("Connect HID Device"));
}
