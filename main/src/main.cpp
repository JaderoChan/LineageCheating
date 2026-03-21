#include "command_line_menu.hpp"
#include "tool_wrapper.hpp"
#include "lineage_cheating.hpp"

int main(int argc, char* argv[])
{
    CommandLineMenu menu;

    menu.setOptionTextAlignment(2);

    menu.addOption("Lineage cheating", lineageCheating, true, false);
    menu.addOption("Select image point", selectImagePointTool);
    menu.addOption("Test HID mouse move", testHidMouseMove);
    menu.addOption("Test HID mouse button", testHidMouseButton);
    menu.addOption("Test HID keyboard", testHidKeyboard);
    menu.addOption("Exit", [](void* menu){
        static_cast<CommandLineMenu*>(menu)->endReceiveInput();
    }, &menu, false, false);

    menu.show();
    menu.startReceiveInput();

    return 0;
}
