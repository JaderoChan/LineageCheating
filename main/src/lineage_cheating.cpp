#include "lineage_cheating.hpp"

#include <opencv2/opencv.hpp>
#include <Processing.NDI.Lib.h>

#include <backend_api/format_string.hpp>

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
        data->menu->setOptionText(data->index, data->sourceName + " (Not Configured)");
        if (recv)
        {
            NDIlib_recv_destroy(recv);
            recv = nullptr;
            data->recv = nullptr;
        }
        data->flag = -1;
    };

    // Create the NDI receiver and connect NDI source.
    printf("Start connect NDI source.\n");
    NDIlib_recv_create_v3_t recvDesc;
    recvDesc.color_format = NDIlib_recv_color_format_BGRX_BGRA;
    NDIlib_recv_instance_t recv = NDIlib_recv_create_v3(&recvDesc);
    if (!recv)
    {
        printf("Failed to create the NDI receiver.\n");
        failHandler(recv);
        return;
    }

    NDIlib_recv_connect(recv, data->ndiSources + data->index);
    data->recv = recv;

    do
    {
        printf("Is major(1)/minor(2):\n");
        if (scanf("%d", &data->flag) != 1 || (data->flag != 1 && data->flag != 2))
        {
            while (getchar() != '\n');
            printf("Please input valid flag. (Press ESC key to stop and return or press other key to retry)\n");
            if (CommandLineMenu::getkey() == 0x1B)
            {
                printf("User cancel configure.\n");
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
            printf("Please input this NDI sources corresponding HID's VID and PID:\n");
            if (scanf("%d %d", &vid, &pid) != 2)
            {
                while (getchar() != '\n');  // Clear input buffer.
                printf("Please input valid PID and VID. (Press ESC key to stop and return or press other key to retry)\n");
                if (CommandLineMenu::getkey() == 0x1B)
                {
                    printf("User cancel configure.\n");
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
            printf("Failed to open the HID (vid: %d, pid: %d).\n", vid, pid);
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
    data->menu->setOptionText(data->index, data->sourceName + (data->flag == 1 ? " (Major)" : " (Minor)"));
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
    data->menu->setOptionText(data->index, data->sourceName + " (Not Configured)");
    data->menu->setOptionCallback(data->index, [data]() { setupCheatingWorker(data); });

    printf("Clean up Cheating Worker successfully.\n");
}

void lineageCheating(bool& needRefresh)
{
    // TODO: input the config file path.
    CheatingConfig cheatingCfg;

    CheatingWorker* worker = nullptr;

    if (!NDIlib_initialize())
    {
        printf("Failed to init NDI.\n");
        return;
    }

    // Find NDI sources.
    NDIlib_find_instance_t ndiFinder = NDIlib_find_create_v2();
    if (!ndiFinder)
    {
        printf("Failed to create NDI finder.\n");
        NDIlib_destroy();
        return;
    }

    uint32_t ndiSourcesNum = 0;
    const NDIlib_source_t* ndiSources = nullptr;
    do
    {
        printf("Looking for NDI sources...\n");
        NDIlib_find_wait_for_sources(ndiFinder, 2000);
        ndiSources = NDIlib_find_get_current_sources(ndiFinder, &ndiSourcesNum);
        if (ndiSourcesNum == 0)
        {
            printf("No NDI source found. (Press ESC key to stop search and return or press other key to retry)\n");
            if (CommandLineMenu::getkey() == 0x1B)
            {
                printf("User cancel NDI source search.\n");
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
        "Find {} NDI sources.\nPlease select the below NDI sources and configure corresponding Cheating Worker.\n",
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
        menu.addOption(sourceName + " (Not Configured)", [&userDatas, i]() { setupCheatingWorker(&userDatas[i]); }, true, false);
    }

    menu.addOption("Start", [&]()
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
                printf("You need configure major and minor devices.\n");
                return;
            }
            else
            {
                worker = new CheatingWorker(major->recv, minor->recv, minor->hid, cheatingCfg);
                worker->run();
                printf("Run successfully, press any key return.\n");
            }
        }
    }, false, false);
    menu.addOption("Stop", [&]()
    {
        if (worker && worker->isRunning())
            worker->stop();
    }, false, false);
    menu.addOption("Refresh", [&]() { needRefresh = true; menu.endReceiveInput(); }, false, false);
    menu.addOption("Exit", [&menu]() { menu.endReceiveInput(); }, false, false);

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
