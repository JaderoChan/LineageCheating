#pragma once

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
    void isRunning() const;

    void configureMasterDevice();
    void configureFootmanDevice();

protected:
    void updateText() override;

    void onSearchNdiSourceButtonClicked(HostFlag flag);

private:
    Ui::AssistProgramOperatePage ui;
    GameData gameData_;
    AssistProgramWorkConfig config_;
    AssistProgram* assistProgram_ = nullptr;
};
