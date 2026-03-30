#include "main_window.h"

#include <Processing.NDI.Lib.h>

MainWindow::MainWindow(QWidget* parent)
    : TrMainWindow(parent)
{
    ui.setupUi(this);

    updateText();
}

void MainWindow::updateText()
{}
