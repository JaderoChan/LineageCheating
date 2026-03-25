#include "tool_wrapper.hpp"

#include <iostream>
#include <string>

#include <backend_api/hid_api.hpp>

#include "command_line_menu.hpp"
#include "image_interactive_utils.hpp"

void selectImagePointTool()
{
    std::string filepath;
    std::cout << "Please input the image file path:\n";
    std::cin >> filepath;

    cv::Mat img = cv::imread(filepath);
    if (img.empty())
    {
        std::cout << "Failed to open the image: " << filepath << std::endl;
        return;
    }
    auto point = selectImagePoint(img);

    std::cout << "Select point: " << point << std::endl;
}

void testHidMouseMove()
{
    int vid = 0, pid = 0;
    std::cout << "Please input the HID VID and PID:\n";
    std::cin >> vid >> pid;

    auto hid = hid::openHID(vid, pid);
    if (!hid || reinterpret_cast<intptr_t>(hid) == -1)
    {
        std::cout << "Failed to open the HID" << std::endl;
        return;
    }

    printf("Please input the HID device resolution (x * y).\n");
    int x = 0, y = 0;
    do
    {
        if (scanf("%d %d", &x, &y) != 2)
        {
            while (getchar() != '\n');
            printf("Please input valid resolution. (Press ESC key to stop and return or press other key to retry)\n");
            if (CommandLineMenu::getkey() == 0x1B)
            {
                printf("User cancel configure.\n");
                return;
            }
        }
        else
        {
            break;
        }
    } while (true);
    if (hid::setResolution(hid, x, y) != 0)
    {
        printf("Failed to set resolution.\n");
        hid::closeHID(hid);
        return;
    }

    do
    {
        int x, y;
        std::cout << "Please input the position for move to:\n";
        std::cin >> x >> y;
        hid::moveMouseTo(hid, x, y);
    } while (CommandLineMenu::getkey() != 0x1B);

    hid::closeHID(hid);
}

void testHidMouseButton()
{
    int vid = 0, pid = 0;
    std::cout << "Please input the HID VID and PID:\n";
    std::cin >> vid >> pid;

    auto hid = hid::openHID(vid, pid);
    if (!hid || reinterpret_cast<intptr_t>(hid) == -1)
    {
        std::cout << "Failed to open the HID" << std::endl;
        return;
    }

    do
    {
        int button;
        std::cout << "Please input the button for mouse press (1: Left, 2: Right, 3: Middle):\n";
        std::cin >> button;
        hid::clickMouseButton(hid, button);
    } while (CommandLineMenu::getkey() != 0x1B);

    hid::closeHID(hid);
}

void testHidKeyboard()
{
    int vid = 0, pid = 0;
    std::cout << "Please input the HID VID and PID:\n";
    std::cin >> vid >> pid;

    auto hid = hid::openHID(vid, pid);
    if (!hid || reinterpret_cast<intptr_t>(hid) == -1)
    {
        std::cout << "Failed to open the HID" << std::endl;
        return;
    }

    // do
    // {
    //     int key;
    //     std::cout << "Please input the key for press:\n";
    //     std::cin >> key;
    //     hid::clickKey(hid, key);
    // } while (CommandLineMenu::getkey() != 0x1B);
    hid::clickKey(hid, VK_F5);

    hid::closeHID(hid);
}
