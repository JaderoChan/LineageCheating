#pragma once

#include <trwidgets/trdialog.h>

#include "ui_edit_assist_program_config_dialog.h"

class EditAssistProgramConfigDialog : public TrDialog
{
public:
    explicit EditAssistProgramConfigDialog(QWidget* parent = nullptr);

protected:
    void updateText() override;

private:
    Ui::EditAssistProgramConfigDialog ui;
};
