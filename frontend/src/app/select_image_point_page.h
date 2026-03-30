#pragma once

#include <trwidgets/trwidget.h>

#include "ui_select_image_point_page.h"

class SelectImagePointPage : public TrWidget
{
public:
    explicit SelectImagePointPage(QWidget* parent = nullptr);

protected:
    void updateText() override;

private:
    Ui::SelectImagePointPage ui;
};
