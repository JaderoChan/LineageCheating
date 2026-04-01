#pragma once

#include <memory>

#include <qtimer.h>

#include <Processing.NDI.Lib.h>

#include <assist_program.hpp>
#include <game_data.hpp>
#include <trwidgets/trwidget.h>

#include "ui_assist_program_operate_page.h"
#include "work_config.h"

class AssistProgramOperatePage : public TrWidget
{
    Q_OBJECT

public:
    enum HostFlag
    {
        Master,
        Footman
    };

    explicit AssistProgramOperatePage(
        const GameData& gameData, const AssistProgramWorkConfig& config, QWidget* parent = nullptr);
    ~AssistProgramOperatePage();

    GameData getGameData() const;
    void setGameData(const GameData& gameData);

    AssistProgramWorkConfig getAssistProgramWorkConfig() const;
    void setAssistProgramWorkConfig(const AssistProgramWorkConfig& config);

    void run();
    void stop();
    bool isRunning() const;

protected:
    void updateText() override;

    void onEditConfigButtonClicked();
    void onNdiConnectButtonClicked(HostFlag flag);
    void onSearchNdiSourceButtonClicked(HostFlag flag);
    void onHidConnectButtonClicked();

private:
    void updateRunningStateWidgets();
    void updateConnectStateWidgets();

    Ui::AssistProgramOperatePage ui;
    GameData gameData_;
    AssistProgramWorkConfig config_;
    std::unique_ptr<AssistProgram> assistProgram_;

    QPixmap redCirclePixmap_;
    QPixmap greenCirclePixmap_;
    QPixmap alertPixmap_;
    QPixmap passPixmap_;

    bool masterNdiConnected_ = false;
    bool footmanNdiConnected_ = false;
    bool footmanHidConnected_ = false;

    NDIlib_recv_instance_t masterRecv_ = nullptr;
    NDIlib_recv_instance_t footmanRecv_ = nullptr;
    hid::HID hid_ = nullptr;

    QTimer runningStateUpdateTimer_;
};
