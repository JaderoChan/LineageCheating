#include "edit_assist_program_config_dialog.h"

#include <qvalidator.h>

EditAssistProgramConfigDialog::EditAssistProgramConfigDialog(const AssistProgramConfig& config, QWidget* parent)
    : TrDialog(parent), config_(config)
{
    ui.setupUi(this);

    // 设置输入控件验证器。
    QIntValidator validator1(0, 5000, this);

    // 设置输入控件初始值

    updateText();
}

AssistProgramConfig EditAssistProgramConfigDialog::execForConfig(bool& isAccept) const
{
    return config_;
}

void EditAssistProgramConfigDialog::updateText()
{
    setWindowTitle(EASYTR("Edit Assist Program Config"));

    ui.treatTimeIntervalTextLabel->setText(EASYTR("Treat Time Interval"));
}
