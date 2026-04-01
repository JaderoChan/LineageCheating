#pragma once

#include <trwidgets/trdialog.h>
#include <assist_program_config.hpp>

#include "ui_edit_assist_program_config_dialog.h"

class EditAssistProgramConfigDialog : public TrDialog
{
public:
    explicit EditAssistProgramConfigDialog(const AssistProgramConfig& config, QWidget* parent = nullptr);

    AssistProgramConfig execForConfig(bool& isAccept) const;

protected:
    void updateText() override;

private:
    Ui::EditAssistProgramConfigDialog ui;
    AssistProgramConfig config_;
};
