#include "edit_assist_program_config_dialog.h"

#include "windows.h"

#include <qvalidator.h>

static QString getFnKeyText(int keyCode)
{
    switch (keyCode)
    {
        case VK_F1:     return "F1";
        case VK_F2:     return "F2";
        case VK_F3:     return "F3";
        case VK_F4:     return "F4";
        case VK_F5:     return "F5";
        case VK_F6:     return "F6";
        case VK_F7:     return "F7";
        case VK_F8:     return "F8";
        case VK_F9:     return "F9";
        case VK_F10:    return "F10";
        case VK_F11:    return "F11";
        case VK_F12:    return "F12";
        default:        return "Not Fn Key";
    }
}

EditAssistProgramConfigDialog::EditAssistProgramConfigDialog(const AssistProgramConfig& config, QWidget* parent)
    : TrDialog(parent), config_(config)
{
    ui.setupUi(this);

    // 设置输入控件限制器。
    auto validator1 = new QIntValidator(0, 5000, this);
    ui.treatTimeIntervalInpputLineEdit->setValidator(validator1);

    auto validator2 = new QIntValidator(50, 5000, this);
    ui.frameGetterTimeoutInputLineEdit->setValidator(validator2);

    auto validator3 = new QIntValidator(0, 60, this);
    ui.cpsInputLineEdit->setValidator(validator3);

    auto validator4 = new QDoubleValidator(0.0, 1.0, 2, this);
    ui.colorConfidenceLineEdit->setValidator(validator4);

    ui.colorConfidenceSlider->setMinimum(0);
    ui.colorConfidenceSlider->setMaximum(100);

    auto validator5 = new QDoubleValidator(0.0, 1.0, 6, this);
    ui.backhomeHpThresholdLineEdit->setValidator(validator5);

    ui.backhomeHpThresholdSlider->setMinimum(0);
    ui.backhomeHpThresholdSlider->setMaximum(100'000'0);

    auto validator6 = new QIntValidator(0, 4000, this);
    ui.debugWindowMaxWidthLineEdit->setValidator(validator6);

    auto validator7 = new QIntValidator(0, 4000, this);
    ui.debugWindowMaxHeightLineEdit->setValidator(validator7);

    // 设置输入控件初始值。
    updateWidgetValue();

    ui.colorConfidenceSlider->installEventFilter(this);
    ui.backhomeHpThresholdSlider->installEventFilter(this);

    // 信号槽
    connect(ui.treatTimeIntervalInpputLineEdit, &QLineEdit::editingFinished, this, [this]()
    { config_.treatTimeInterval = ui.treatTimeIntervalInpputLineEdit->text().toUInt(); });
    connect(ui.frameGetterTimeoutInputLineEdit, &QLineEdit::editingFinished, this, [this]()
    { config_.frameGetterTimeout = ui.frameGetterTimeoutInputLineEdit->text().toUInt(); });
    connect(ui.cpsInputLineEdit, &QLineEdit::editingFinished, this, [this]()
    { config_.cps = ui.cpsInputLineEdit->text().toUInt(); });

    connect(ui.colorConfidenceSlider, &QSlider::valueChanged, this, [this](int value)
    {
        config_.colorConfidence = static_cast<double>(value) / 100.0;
        updateWidgetValue();
    });
    connect(ui.colorConfidenceLineEdit, &QLineEdit::editingFinished, this, [this]()
    {
        config_.colorConfidence = ui.colorConfidenceLineEdit->text().toDouble();
        updateWidgetValue();
    });

    connect(ui.backhomeHpThresholdSlider, &QSlider::valueChanged, this, [this](int value)
    {
        config_.backHomeHpThreshold = static_cast<double>(value) / 100'000'0;
        updateWidgetValue();
    });
    connect(ui.backhomeHpThresholdLineEdit, &QLineEdit::editingFinished, this, [this]()
    {
        config_.backHomeHpThreshold = ui.backhomeHpThresholdLineEdit->text().toDouble();
        updateWidgetValue();
    });

    connect(ui.debugLogOutputCheckBox, &QCheckBox::stateChanged, this, [this]()
    { config_.outputLog= ui.debugLogOutputCheckBox->isChecked(); });
    connect(ui.debugWindowDisplayCheckBox, &QCheckBox::stateChanged, this, [this]()
    { config_.showDebugWindow = ui.debugWindowDisplayCheckBox->isChecked(); });
    connect(ui.limitDebugWindowSizeCheckBox, &QCheckBox::stateChanged, this, [this]()
    { config_.limitDebugWindowSize= ui.limitDebugWindowSizeCheckBox->isChecked(); });

    connect(ui.debugWindowMaxWidthLineEdit, &QLineEdit::editingFinished, this, [this]()
    { config_.debugWindowMaxWidth = ui.debugWindowMaxWidthLineEdit->text().toUInt(); });
    connect(ui.debugWindowMaxHeightLineEdit, &QLineEdit::editingFinished, this, [this]()
    { config_.debugWindowMaxHeight = ui.debugWindowMaxHeightLineEdit->text().toUInt(); });

    connect(ui.confirmButton, &QPushButton::clicked, this, &EditAssistProgramConfigDialog::accept);
    connect(ui.cancelButton, &QPushButton::clicked, this, &EditAssistProgramConfigDialog::reject);

    updateText();
}

AssistProgramConfig EditAssistProgramConfigDialog::execForConfig(bool& isAccept)
{
    int ret = exec();
    if (ret == Accepted)
    {
        isAccept = true;
        return config_;
    }
    else
    {
        isAccept = false;
        return AssistProgramConfig();
    }
}

void EditAssistProgramConfigDialog::updateText()
{
    setWindowTitle(EASYTR("Edit Assist Program Config"));

    ui.treatTimeIntervalTextLabel->setText(EASYTR("Treat Time Interval"));
    ui.frameGetterTimeoutTextLabel->setText(EASYTR("Frame Getter Timeout"));
    ui.cpsTextLabel->setText(EASYTR("Click Per Second"));

    ui.hotkeyGroupBox->setTitle(EASYTR("Hotkey"));
    ui.treatMasterHotkeyTextLabel->setText(EASYTR("Treat Master Hotkey"));
    ui.treatFootmanHotkeyTextLabel->setText(EASYTR("Treat Footman Hotkey"));
    ui.backhomeFootmanHotkeyTextLabel->setText(EASYTR("Backhome Footman Hotkey"));

    ui.colorConfidenceTextLabel->setText(EASYTR("Color Confidence"));
    ui.backhomeHpThresholdTextLabel->setText(EASYTR("Footman Backhome HP threshold"));

    ui.debugGroupBox->setTitle(EASYTR("Debug"));
    ui.debugLogOutputCheckBox->setText(EASYTR("Enabel Debug Log Ouput"));
    ui.debugWindowDisplayCheckBox->setText(EASYTR("Show Debug Window"));
    ui.limitDebugWindowSizeCheckBox->setText(EASYTR("Limit Debug Window Size"));
    ui.debugWindowMaxWidthTextLabel->setText(EASYTR("Max Width"));
    ui.debugWindowMaxHeightTextLabel->setText(EASYTR("Max Height"));

    ui.confirmButton->setText(EASYTR("Confirm"));
    ui.cancelButton->setText(EASYTR("Cancel"));
}

bool EditAssistProgramConfigDialog::eventFilter(QObject* obj, QEvent* event)
{
    // 禁用滑条的鼠标滚轮响应。
    if (obj == ui.colorConfidenceSlider || obj == ui.backhomeHpThresholdSlider)
    {
        if (event->type() == QEvent::Wheel)
            return true;
    }

    return TrDialog::eventFilter(obj, event);
}

void EditAssistProgramConfigDialog::updateWidgetValue()
{
    ui.treatTimeIntervalInpputLineEdit->setText(QString::number(config_.treatTimeInterval));
    ui.frameGetterTimeoutInputLineEdit->setText(QString::number(config_.frameGetterTimeout));
    ui.cpsInputLineEdit->setText(QString::number(config_.cps));

    ui.treatMasterHotkeyInputer->setText(getFnKeyText(config_.masterTreatKey));
    ui.treatFootmanHotkeyInputer->setText(getFnKeyText(config_.footmanTreatKey));
    ui.backhomeFootmanHotkeyInputer->setText(getFnKeyText(config_.backHomeKey));

    ui.colorConfidenceSlider->setValue(config_.colorConfidence * 100);
    ui.colorConfidenceLineEdit->setText(QString::number(config_.colorConfidence));

    ui.backhomeHpThresholdSlider->setValue(config_.backHomeHpThreshold * 10e6);
    ui.backhomeHpThresholdLineEdit->setText(QString::number(config_.backHomeHpThreshold));

    ui.debugLogOutputCheckBox->setChecked(config_.outputLog);
    ui.debugWindowDisplayCheckBox->setChecked(config_.showDebugWindow);
    ui.limitDebugWindowSizeCheckBox->setChecked(config_.limitDebugWindowSize);

    ui.debugWindowMaxWidthLineEdit->setText(QString::number(config_.debugWindowMaxWidth));
    ui.debugWindowMaxHeightLineEdit->setText(QString::number(config_.debugWindowMaxHeight));
}
