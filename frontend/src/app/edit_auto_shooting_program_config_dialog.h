#pragma once

#include <trwidgets/trdialog.h>

#include "ui_edit_auto_shooting_program_config_dialog.h"

class EditAutoShootingProgramConfigDialog : public TrDialog
{
public:
    explicit EditAutoShootingProgramConfigDialog(QWidget* parent = nullptr);

protected:
    void updateText() override;

private:
    Ui::EditAutoShootingProgramConfigDialog ui;
};
