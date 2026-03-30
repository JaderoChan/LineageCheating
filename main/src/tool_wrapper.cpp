#include "tool_wrapper.hpp"

#include <iostream>
#include <string>

#include <hid_api.hpp>

#include "command_line_menu.hpp"
#include "image_interactive_utils.hpp"

void selectImagePointTool()
{
    std::string filepath;
    std::cout << "请输入图像文件路径：\n";
    std::cin >> filepath;

    cv::Mat img = cv::imread(filepath);
    if (img.empty())
    {
        std::cout << "读取下列文件失败：" << filepath << std::endl;
        return;
    }
    auto point = selectImagePoint(img);

    std::cout << "所选点坐标：" << point << std::endl;
}

void cropImageByRectTool()
{
    cropImageByRect();
}

void testHidMouseMove()
{
    int vid = 0, pid = 0;
    std::cout << "请输入 HID 设备的 VID 与 PID：\n";
    std::cin >> vid >> pid;

    auto hid = hid::openHID(vid, pid);
    if (!hid || reinterpret_cast<intptr_t>(hid) == -1)
    {
        std::cout << "无法打开 HID 设备。" << std::endl;
        return;
    }

    printf("请输入 HID 设备屏幕分辨率（Width * Height）：\n");
    int x = 0, y = 0;
    do
    {
        if (scanf("%d %d", &x, &y) != 2)
        {
            while (getchar() != '\n');
            printf("请输入有效分辨率。（按 ESC 键退出或按其他键重试）\n");
            if (CommandLineMenu::getkey() == 0x1B)
            {
                printf("用户取消操作。\n");
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
        printf("设置 HID 设备屏幕分辨率失败。\n");
        hid::closeHID(hid);
        return;
    }

    do
    {
        int x, y;
        std::cout << "请输入移动位置坐标：\n";
        std::cin >> x >> y;
        hid::moveMouseTo(hid, x, y);
    } while (CommandLineMenu::getkey() != 0x1B);

    hid::closeHID(hid);
}

void testHidMouseButton()
{
    int vid = 0, pid = 0;
    std::cout << "请输入 HID 设备的 VID 与 PID：\n";
    std::cin >> vid >> pid;

    auto hid = hid::openHID(vid, pid);
    if (!hid || reinterpret_cast<intptr_t>(hid) == -1)
    {
        std::cout << "无法打开 HID 设备。" << std::endl;
        return;
    }

    do
    {
        int button;
        std::cout << "请输入要点击的鼠标按钮（1：鼠标左键；2：鼠标右键；3：鼠标中键）：\n";
        std::cin >> button;
        hid::clickMouseButton(hid, button);
    } while (CommandLineMenu::getkey() != 0x1B);

    hid::closeHID(hid);
}

void testHidKeyboard()
{
    int vid = 0, pid = 0;
    std::cout << "请输入 HID 设备的 VID 与 PID：\n";
    std::cin >> vid >> pid;

    auto hid = hid::openHID(vid, pid);
    if (!hid || reinterpret_cast<intptr_t>(hid) == -1)
    {
        std::cout << "无法打开 HID 设备。" << std::endl;
        return;
    }

    do
    {
        int key;
        std::cout << "请输入需按下的键：\n";
        std::cin >> key;
        hid::clickKey(hid, key);
    } while (CommandLineMenu::getkey() != 0x1B);

    hid::closeHID(hid);
}
