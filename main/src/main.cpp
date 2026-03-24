#include "command_line_menu.hpp"
#include "tool_wrapper.hpp"
#include "lineage_cheating.hpp"

int main(int argc, char* argv[])
{
    CommandLineMenu menu;

    menu.setOptionTextAlignment(2);

    bool needRefresh = false;
    menu.addOption("Lineage cheating", [&needRefresh]()
    {
        do
        {
            needRefresh = false;
            lineageCheating(needRefresh);
        } while (needRefresh);
    }, true, false);
    menu.addOption("Select image point", selectImagePointTool);
    menu.addOption("Test HID mouse move", testHidMouseMove);
    menu.addOption("Test HID mouse button", testHidMouseButton);
    menu.addOption("Test HID keyboard", testHidKeyboard);
    menu.addOption("Exit", [&menu](){ menu.endReceiveInput(); }, false, false);

    system("chcp 65001");
    menu.show();
    menu.startReceiveInput();

    return 0;
}
