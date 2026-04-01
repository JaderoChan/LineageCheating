#pragma once

#include <qevent.h>

#include <trwidgets/trdialog.h>
#include <assist_program_config.hpp>

#include "ui_edit_assist_program_config_dialog.h"

class EditAssistProgramConfigDialog : public TrDialog
{
public:
    explicit EditAssistProgramConfigDialog(const AssistProgramConfig& config, QWidget* parent = nullptr);

    AssistProgramConfig execForConfig(bool& isAccept);

protected:
    void updateText() override;
    bool eventFilter(QObject* obj, QEvent* event) override;

    void updateWidgetValue();

private:
    Ui::EditAssistProgramConfigDialog ui;
    AssistProgramConfig config_;
};
