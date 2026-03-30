#include "command_line_menu.hpp"
#include "tool_wrapper.hpp"
#include "lineage_cheating.hpp"

int main(int argc, char* argv[])
{
    CommandLineMenu menu;

    menu.setOptionTextAlignment(2);

    bool needRefresh = false;
    menu.addOption("Lineage 辅助程序", [&needRefresh]()
    {
        do
        {
            needRefresh = false;
            lineageCheating(needRefresh);
        } while (needRefresh);
    }, true, false);
    menu.addOption("选取图像坐标位置", selectImagePointTool);
    menu.addOption("裁切图像", cropImageByRectTool);
    menu.addOption("测试 HID 鼠标移动", testHidMouseMove);
    menu.addOption("测试 HID 鼠标按键", testHidMouseButton);
    menu.addOption("测试 HID 键盘", testHidKeyboard);
    menu.addOption("退出", [&menu](){ menu.endReceiveInput(); }, false, false);

    system("chcp 65001");
    menu.show();
    menu.startReceiveInput();

    return 0;
}
