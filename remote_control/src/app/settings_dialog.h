#pragma once

#include <trwidgets/trdialog.h>

#include "ui_settings_dialog.h"
#include "settings.h"

class SettingsDialog : public TrDialog
{
public:
    explicit SettingsDialog(const Settings& settings, QWidget* parent = nullptr);

    Settings execForSettings(bool& isConfirm /**< [in, out] */);

protected:
    void updateText() override;

private:
    Ui::SettingsDialog ui;
    Settings settings_;
};
