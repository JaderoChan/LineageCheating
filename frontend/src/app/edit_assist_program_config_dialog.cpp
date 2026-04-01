#include "edit_assist_program_config_dialog.h"

EditAssistProgramConfigDialog::EditAssistProgramConfigDialog(const AssistProgramConfig& config, QWidget* parent)
    : TrDialog(parent), config_(config)
{
    ui.setupUi(this);


    updateText();
}

AssistProgramConfig EditAssistProgramConfigDialog::execForConfig(bool& isAccept) const
{
    return config_;
}

void EditAssistProgramConfigDialog::updateText()
{}
