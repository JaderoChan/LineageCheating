#include "edit_assist_program_config_dialog.h"

EditAssistProgramConfigDialog::EditAssistProgramConfigDialog(QWidget* parent)
    : TrDialog(parent)
{
    ui.setupUi(this);

    updateText();
}

void EditAssistProgramConfigDialog::updateText()
{}
