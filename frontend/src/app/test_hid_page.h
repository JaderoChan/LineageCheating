#pragma once

#include <trwidgets/trwidget.h>

#include "ui_test_hid_page.h"

class TestHidPage : public TrWidget
{
public:
    explicit TestHidPage(QWidget* parent = nullptr);

protected:
    void updateText() override;

private:
    Ui::TestHidPage ui;
};
