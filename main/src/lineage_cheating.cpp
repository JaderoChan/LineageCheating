#include "lineage_cheating.hpp"

#include <opencv2/opencv.hpp>
#include <Processing.NDI.Lib.h>

#include <format_string.hpp>
#include <assist_program.hpp>

#include "command_line_menu.hpp"

// #define DISABLE_HID

struct UserData
{
    // In
    uint32_t index;
    const NDIlib_source_t* ndiSources;
    std::string sourceName;

    // Out
    bool configureSuccess;
    NDIlib_recv_instance_t recv;
    hid::HID hid;
    int flag;

    // In, Out
    CommandLineMenu* menu;
};

static void setupCheatingWorker(UserData* data);
static void cleanupCheatingWorker(UserData* data);

void setupCheatingWorker(UserData* data)
{
    auto failHandler = [&](NDIlib_recv_instance_t& recv)
    {
        data->configureSuccess = false;
        data->menu->setOptionText(data->index, data->sourceName + " （未配置）");
        if (recv)
        {
            NDIlib_recv_destroy(recv);
            recv = nullptr;
            data->recv = nullptr;
        }
        data->flag = -1;
    };

    // Create the NDI receiver and connect NDI source.
    printf("开始连接 NDI 设备源。\n");
    NDIlib_recv_create_v3_t recvDesc;
    recvDesc.color_format = NDIlib_recv_color_format_BGRX_BGRA;
    NDIlib_recv_instance_t recv = NDIlib_recv_create_v3(&recvDesc);
    if (!recv)
    {
        printf("创建 NDI 接收器失败。\n");
        failHandler(recv);
        return;
    }

    NDIlib_recv_connect(recv, data->ndiSources + data->index);
    data->recv = recv;

    do
    {
        printf("请输入此设备类型（战斗机：1；辅助机：2）：\n");
        if (scanf("%d", &data->flag) != 1 || (data->flag != 1 && data->flag != 2))
        {
            while (getchar() != '\n');
            printf("请输入有效的设备类型。（按 ESC 键退出或按任意键重试）\n");
            if (CommandLineMenu::getkey() == 0x1B)
            {
                printf("用户取消操作。\n");
                failHandler(recv);
                return;
            }
        }
        else
        {
            break;
        }
    } while (true);

    if (data->flag == 2)
    {
    #ifndef DISABLE_HID
        // Input the HID's VID and PID.
        int pid = 0, vid = 0;
        do
        {
            printf("请输入此 NDI 源对应的 HID 设备的 VID 与 PID：\n");
            if (scanf("%d %d", &vid, &pid) != 2)
            {
                while (getchar() != '\n');  // Clear input buffer.
                printf("请输入有效的 PID 与 VID。（按 ESC 键退出或按任意键重试）\n");
                if (CommandLineMenu::getkey() == 0x1B)
                {
                    printf("用户取消操作。\n");
                    failHandler(recv);
                    return;
                }
            }
            else
            {
                break;
            }
        } while (true);

        // Open the HID by given PID and VID.
        auto hid = hid::openHID(vid, pid);
        if (!hid || reinterpret_cast<intptr_t>(hid) == -1)
        {
            printf("无法打开 HID 设备（VID：%d，PID：%d）。\n", vid, pid);
            failHandler(recv);
            return;
        }
    #else
        hid::HID hid = nullptr;
    #endif // !DISABLE_HID
        data->hid = hid;
    }

    // Set menu option and user data.
    data->configureSuccess = true;
    data->menu->setOptionText(data->index, data->sourceName + (data->flag == 1 ? " （战斗机）" : " （辅助机）"));
    data->menu->setOptionCallback(data->index, [data]() {cleanupCheatingWorker(data); });
}

void cleanupCheatingWorker(UserData* data)
{
#ifndef DISABLE_HID
    if (data->hid)
    {
        hid::releaseAllKey(data->hid);
        hid::releaseAllMouseButton(data->hid);

        // Close HID device.
        hid::closeHID(data->hid);
    }
#endif // !DISABLE_HID

    // Destroy NDI receiver.
    if (data->recv)
    {
        NDIlib_recv_destroy(data->recv);
        data->recv = nullptr;
    }

    // Reset menu option and user data.
    data->configureSuccess = false;
    data->menu->setOptionText(data->index, data->sourceName + " （未配置）");
    data->menu->setOptionCallback(data->index, [data]() { setupCheatingWorker(data); });

    printf("成功退出。\n");
}

void lineageCheating(bool& needRefresh)
{
    // TODO: input the config file path.
    GameData gameData;
    AssistProgramConfig config;
    try
    {
        gameData = GameData::fromFile("./game_data.json");
    }
    catch (const std::exception& e)
    {
        printf("Failed to read game data: %s", e.what());
        return;
    }

    AssistProgram* worker = nullptr;

    if (!NDIlib_initialize())
    {
        printf("初始化 NDI 失败。\n");
        return;
    }

    // Find NDI sources.
    NDIlib_find_instance_t ndiFinder = NDIlib_find_create_v2();
    if (!ndiFinder)
    {
        printf("创建 NDI 接收器失败。\n");
        NDIlib_destroy();
        return;
    }

    uint32_t ndiSourcesNum = 0;
    const NDIlib_source_t* ndiSources = nullptr;
    do
    {
        printf("查找 NDI 源中……\n");
        NDIlib_find_wait_for_sources(ndiFinder, 2000);
        ndiSources = NDIlib_find_get_current_sources(ndiFinder, &ndiSourcesNum);
        if (ndiSourcesNum == 0)
        {
            printf("未发现任何 NDI 源。（按 ESC 键退出或按任意键重试）\n");
            if (CommandLineMenu::getkey() == 0x1B)
            {
                printf("用户取消操作。\n");
                NDIlib_find_destroy(ndiFinder);
                NDIlib_destroy();
                return;
            }
        }
        else
        {
            break;
        }
    } while (true);

    // Set the menu.
    CommandLineMenu menu;
    menu.setOptionTextAlignment(2);
    menu.setTopText(formatString(
        "发现 {} 个 NDI 源。\n请选择下列 NDI 源配置其类型。\n",
        ndiSourcesNum));

    std::vector<UserData> userDatas;
    userDatas.reserve(ndiSourcesNum);
    for (uint32_t i = 0; i < ndiSourcesNum; ++i)
    {
        std::string sourceName = (ndiSources + i)->p_ndi_name;

        UserData data;
        data.index = i;
        data.ndiSources = ndiSources;
        data.sourceName = sourceName;
        data.configureSuccess = false;
        data.recv = nullptr;
        data.hid = nullptr;
        data.flag = -1;
        data.menu = &menu;

        userDatas.emplace_back(data);
        menu.addOption(sourceName + " （未配置）", [&userDatas, i]() { setupCheatingWorker(&userDatas[i]); }, true, false);
    }

    menu.addOption("开始", [&]()
    {
        if (worker && !worker->isRunning())
            worker->run();
        if (!worker)
        {
            UserData* major = nullptr;
            UserData* minor = nullptr;
            for (auto& data : userDatas)
            {
                if (data.flag == 1)
                    major = &data;
                else if (data.flag == 2)
                    minor = &data;
            }

            if (major == nullptr || minor == nullptr)
            {
                printf("你需要配置一个战斗机与辅助机\n");
                return;
            }
            else
            {
                worker = new AssistProgram(major->recv, minor->recv, minor->hid, gameData, config);
                worker->run();
                printf("运行成功，按任意键返回。\n");
            }
        }
    }, false, false);
    menu.addOption("停止", [&]()
    {
        if (worker && worker->isRunning())
            worker->stop();
    }, false, false);
    menu.addOption("刷新", [&]() { needRefresh = true; menu.endReceiveInput(); }, false, false);
    menu.addOption("退出", [&menu]() { menu.endReceiveInput(); }, false, false);

    menu.show();
    menu.startReceiveInput();

    // Cleanup
    if (worker)
    {
        worker->stop();
        delete worker;
        worker = nullptr;
    }

    for (auto& data : userDatas)
        cleanupCheatingWorker(&data);

    NDIlib_find_destroy(ndiFinder);
    NDIlib_destroy();

    cv::destroyAllWindows();
}
