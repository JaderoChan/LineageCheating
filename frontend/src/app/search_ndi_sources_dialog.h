#pragma once

#include <trwidgets/trdialog.h>

#include "ui_search_ndi_sources_dialog.h"

class SearchNdiSourcesDialog : public TrDialog
{
public:
    explicit SearchNdiSourcesDialog(QWidget* parent = nullptr);

protected:
    void updateText() override;

private:
    Ui::SearchNdiSourcesDialog ui;
};
