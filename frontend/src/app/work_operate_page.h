#pragma once

#include <assist_program.hpp>

#include <trwidgets/trwidget.h>

#include "ui_work_operate_page.h"
#include "work_config.h"

class WorkOperatePage : public TrWidget
{
    Q_OBJECT

public:
    explicit WorkOperatePage(const WorkConfig& config, QWidget* parent = nullptr);
    ~WorkOperatePage();

    WorkConfig getWorkConfig() const;
    void setWorkConfig(const WorkConfig& config);

    void run();
    void stop();
    void isRunning() const;

    void configureMasterDevice();
    void configureFootmanDevice();

protected:
    void updateText() override;

private:
    Ui::WorkOperatePage ui;
    WorkConfig config_;
    AssistProgram* assistProgram_ = nullptr;
};
