#pragma once

#include <trwidgets/trmainwindow.h>

#include "ui_main_window.h"

class MainWindow : public TrMainWindow
{
public:
    explicit MainWindow(QWidget* parent = nullptr);

private:
    Ui::MainWindow ui;
};
