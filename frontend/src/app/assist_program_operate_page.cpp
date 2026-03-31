#include "assist_program_operate_page.h"

#include <qvalidator.h>

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

    // 信号槽
    connect(ui.masterNdiConnectButton, &QPushButton::clicked,
        this, [this]() { onNdiConnectButtonClicked(Master); });
    connect(ui.footmanNdiConnectButton, &QPushButton::clicked,
        this, [this]() { onNdiConnectButtonClicked(Footman); });
    connect(ui.footmanHidConnectButton, &QPushButton::clicked,
        this, &AssistProgramOperatePage::onHidConnectButtonClicked);

    updateText();
    updateStateIconAndText();
}

AssistProgramOperatePage::~AssistProgramOperatePage()
{
    stop();
}

GameData AssistProgramOperatePage::getGameData() const
{
    return gameData_;
}

void AssistProgramOperatePage::setGameData(const GameData& gameData)
{
    gameData_ = gameData;
}

AssistProgramWorkConfig AssistProgramOperatePage::getAssistProgramWorkConfig() const
{
    return config_;
}

void AssistProgramOperatePage::setAssistProgramWorkConfig(const AssistProgramWorkConfig& config)
{
    config_ = config;
}

void AssistProgramOperatePage::run()
{
    if (isRunning())
        return;
}

void AssistProgramOperatePage::stop()
{
    if (!isRunning())
        return;
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
    ui.masterSearchNdiSourceButton->setToolTip(EASYTR("Search and select NDI source"));
    ui.masterNdiConnectButton->setText(EASYTR("Connect NDI Source"));

    // Footman Host Group Box
    ui.footmanConfigureGroupBox->setTitle(EASYTR("Footman Host"));
    ui.footmanNdiTextLabel->setText(EASYTR("NDI Source"));
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

void AssistProgramOperatePage::onNdiConnectButtonClicked(HostFlag flag)
{}

void AssistProgramOperatePage::onSearchNdiSourceButtonClicked(HostFlag flag)
{}

void AssistProgramOperatePage::onHidConnectButtonClicked()
{}

void AssistProgramOperatePage::updateStateIconAndText()
{
    if (isRunning())
    {
        ui.runningStateIcon->setPixmap(greenCirclePixmap_);
        ui.runningStateTextLabel->setText(EASYTR("(Is Running)"));
    }
    else
    {
        ui.runningStateIcon->setPixmap(redCirclePixmap_);
        ui.runningStateTextLabel->setText(EASYTR("(Not Running)"));
    }

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
