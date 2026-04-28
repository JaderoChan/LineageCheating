#include "about_dialog.h"

#include <config.h>

AboutDialog::AboutDialog(QWidget* parent)
{
    ui.setupUi(this);

    updateText();
}

void AboutDialog::updateText()
{
    setWindowTitle(EASYTR("About"));

    ui.versionTextLabel->setText(EASYTR("Version: "));
    ui.versionLabel->setText(APP_VERSION);
}
