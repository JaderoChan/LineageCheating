#pragma once

#include <trwidgets/trwidget.h>

#include "ui_select_screen_color_page.h"

class SelectScreenColorPage : public TrWidget
{
public:
    explicit SelectScreenColorPage(QWidget* parent = nullptr);

protected:
    void updateText() override;

private:
    Ui::SelectScreenColorPage ui;
};
