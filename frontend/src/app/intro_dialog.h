#pragma once

#include <trwidgets/trdialog.h>

#include "ui_intro_dialog.h"

class IntroDialog : public TrDialog
{
public:
    explicit IntroDialog(QWidget* parent = nullptr);

    bool execForAccess();

    void updateText() override;

private:
    bool checkPassword(const QString& password);

    Ui::IntroDialog ui;
};
