#pragma once

#include <qlist.h>
#include <qmovie.h>
#include <qthread.h>
#include <qvariant.h>

#include <trwidgets/trdialog.h>

#include "ui_search_ndi_sources_dialog.h"
#include "ndi_finder_worker.h"

class SearchNdiSourcesDialog : public TrDialog
{
public:
    explicit SearchNdiSourcesDialog(QWidget* parent = nullptr);
    ~SearchNdiSourcesDialog();

    QVariant getSelectedNdiSource();

protected:
    void updateText() override;

    void onRefreshButtonClicked();
    void onConfirmButtonClicked();
    void onCancelButtonClicked();

private:
    void refresh();
    void onFindFinished(const QList<QPair<QString, QString>>& sources, bool success);

    Ui::SearchNdiSourcesDialog ui;
    QThread workerThread_;
    QMovie loadingMovie_;
    QString selectedSourceName_;
};
