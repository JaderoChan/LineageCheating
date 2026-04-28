#include <qapplication.h>

#include <Processing.NDI.Lib.h>
#include <easy_translate.hpp>

#include <config.h>
#include <utils/debug_output.h>

#include "logo_icon.h"
#include "intro_dialog.h"
#include "main_window.h"
#include "settings.h"

int main(int argc, char* argv[])
{
    // 设置程序全局属性
    QApplication a(argc, argv);
    a.setOrganizationDomain(APP_ORGANIZATION_DOMAIN);
    a.setOrganizationName(APP_ORGANIZATION);
    a.setApplicationName(APP_TITLE);
    a.setApplicationVersion(APP_VERSION);
    a.setWindowIcon(getLogoIcon());

    // 设置语言
    {
        Settings settings = loadSettings();
        setLanguage(settings.language);
    }

    {
        IntroDialog dlg;
        if (!dlg.execForAccess())
        {
            debugOut(qCritical(), "Password error, user exit program.");
            return 1;
        }
    }

    MainWindow wgt;
    wgt.show();

    if (!NDIlib_initialize())
    {
        debugOut(qCritical(), "Failed to initialize NDI library.");
        return -1;
    }

    int ret = a.exec();

    // 更新翻译文件（实际上由编译选项 `UPDATE_TRANSLATIONS_FILES` 决定是否真正更新）
    easytr::updateTranslationsFiles();

    NDIlib_destroy();

    return ret;
}
