#pragma once

#include <trwidgets/trwidget.h>

#include "ui_work_operate_page.h"

class WorkOperatePage : public TrWidget
{
public:
    explicit WorkOperatePage(QWidget* parent = nullptr);

protected:
    void updateText() override;

private:
    Ui::WorkOperatePage ui;
};
