#pragma once

#include <qevent.h>
#include <qlist.h>
#include <qpushbutton.h>

#include <trwidgets/trmainwindow.h>

#include "ui_main_window.h"
#include "work_operate_page.h"

class MainWindow : public TrMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

    void addWorkOperatePage(WorkOperatePage* page, bool jumpTo);
    void addWorkOperatePage(bool jumpTo);
    void removeWorkOperatePage(int index);
    void removeWorkOperatePage(WorkOperatePage* page);

protected:
    void updateText() override;
    bool eventFilter(QObject* obj, QEvent* event) override;

    void onTabCloseRequested(int index);

private:
    void startRenameTab(int index);

    Ui::MainWindow ui;
    QPushButton* tabWidgetAddBtn_;
};
