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
    int hidNo = 0;
    std::cout << "Please input the HID number:\n";
    std::cin >> hidNo;

    auto hid = hid::openHID(0x0001, hidNo);
    if (!hid || reinterpret_cast<intptr_t>(hid) == -1)
    {
        std::cout << "Failed to open the HID " << hidNo << std::endl;
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
    int hidNo = 0;
    std::cout << "Please input the HID number:\n";
    std::cin >> hidNo;

    auto hid = hid::openHID(0x0001, hidNo);
    if (!hid || reinterpret_cast<intptr_t>(hid) == -1)
    {
        std::cout << "Failed to open the HID " << hidNo << std::endl;
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
    int hidNo = 0;
    std::cout << "Please input the HID number:\n";
    std::cin >> hidNo;

    auto hid = hid::openHID(0x0001, hidNo);
    if (!hid || reinterpret_cast<intptr_t>(hid) == -1)
    {
        std::cout << "Failed to open the HID " << hidNo << std::endl;
        return;
    }

    do
    {
        int key;
        std::cout << "Please input the key for press:\n";
        std::cin >> key;
        hid::clickKey(hid, key);
    } while (CommandLineMenu::getkey() != 0x1B);

    hid::closeHID(hid);
}
