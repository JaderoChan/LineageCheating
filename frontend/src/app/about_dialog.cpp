#include "about_dialog.h"

#include <config.h>

AboutDialog::AboutDialog(QWidget* parent)
    : TrDialog(parent)
{
    ui.setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);

    updateText();
}

void AboutDialog::updateText()
{
    setWindowTitle(EASYTR("About"));

    ui.versionTextLabel->setText(EASYTR("Version: "));
    ui.versionLabel->setText(APP_VERSION);
}
