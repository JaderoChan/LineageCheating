#include "assist_program_operate_page.h"

AssistProgramOperatePage::AssistProgramOperatePage(
    const GameData& gameData, const AssistProgramWorkConfig& config, QWidget* parent)
    : TrWidget(parent), gameData_(gameData), config_(config)
{
    ui.setupUi(this);

    QPixmap redCircle = QPixmap(":/icons/red_circle.png");
    ui.runningStateIcon->setPixmap(redCircle);
    QPixmap alert = QPixmap(":/icons/alert.png");
    ui.masterNdiStateIcon->setPixmap(alert);
    ui.footmanNdiStateIcon->setPixmap(alert);
    ui.footmanHidStateIcon->setPixmap(alert);

    ui.masterNdiSourceNameLabel->setText(config_.masterNdiSourceName);
    ui.footmanNdiSourceNameLabel->setText(config_.footmanNdiSourceName);
    ui.footmanHidVidInputLineEdit->setText(QString::number(config_.footmanHidInfo.vid));
    ui.footmanHidPidInputLineEdit->setText(QString::number(config_.footmanHidInfo.pid));

    updateText();
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
{}

void AssistProgramOperatePage::stop()
{}

void AssistProgramOperatePage::isRunning() const
{}

void AssistProgramOperatePage::configureMasterDevice()
{}

void AssistProgramOperatePage::configureFootmanDevice()
{}

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

void AssistProgramOperatePage::onSearchNdiSourceButtonClicked(HostFlag flag)
{}
