#pragma once

#include <qevent.h>
#include <qmap.h>
#include <qpushbutton.h>

#include <trwidgets/trmainwindow.h>

#include "ui_main_window.h"
#include "assist_program_operate_page.h"
#include "work_config.h"

class MainWindow : public TrMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    void addTabPage(bool jumpTo);
    void removeTabPage(int index);

protected:
    void updateText() override;
    bool eventFilter(QObject* obj, QEvent* event) override;

    void onTabCloseRequested(int index);

private:
    void cleanupWorkPage(QWidget* wgt);
    void startRenameTab(int index);

    Ui::MainWindow ui;
    QPushButton* tabWidgetAddBtn_;
    QMap<QWidget*, WorkConfig> pageAndConfigMap_;
};
