#include "lineage_cheating.hpp"

#include <Processing.NDI.Lib.h>

#include "command_line_menu.hpp"
#include "format_string.hpp"

struct UserData
{
    // In
    uint32_t index;
    const NDIlib_source_t* ndiSources;
    std::string sourceName;
    CheatingConfig cheatingCfg;
    DebugModeConfig debugModeCfg;

    // Out
    CheatingWorker* worker;
    bool configureSuccess;
    NDIlib_recv_instance_t recv;
    hid::HID hid;

    // In, Out
    CommandLineMenu* menu;
};

static void setupCheatingWorker(void* userData);
static void cleanupCheatingWorker(void* userData);

void setupCheatingWorker(void* userData)
{
    UserData* data = static_cast<UserData*>(userData);

    auto failHandler = [&](NDIlib_recv_instance_t& recv)
    {
        data->worker = nullptr;
        data->configureSuccess = false;
        data->menu->setOptionText(data->index, data->sourceName + " (Not Configured)");
        if (recv)
        {
            NDIlib_recv_destroy(recv);
            recv = nullptr;
        }
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
    // if (!hid || reinterpret_cast<intptr_t>(hid) == -1)
    // {
    //     printf("Failed to open the HID (vid: %d, pid: %d).\n", vid, pid);
    //     failHandler(recv);
    //     return;
    // }

    // Start Cheating Worker.
    auto* worker = new CheatingWorker(recv, hid, data->cheatingCfg, data->debugModeCfg);
    worker->run();

    // Set menu option and user data.
    data->configureSuccess = true;
    data->worker = worker;
    data->hid = hid;
    data->menu->setOptionText(data->index, data->sourceName + " (Configured)");
    data->menu->setOptionCallback(data->index, cleanupCheatingWorker, userData);
}

void cleanupCheatingWorker(void* userData)
{
    UserData* data = static_cast<UserData*>(userData);

    if (!data->configureSuccess)
        return;

    // Stop Cheating Worker.
    if (data->worker)
    {
        printf("Wait Cheating Worker exit...\n");
        data->worker->stop();
        printf("Success exit the Cheating Worker.\n");

        delete data->worker;
        data->worker = nullptr;
    }

    // Close HID device.
    // TODO
    // hid::closeHID(data->hid);

    // Destroy NDI receiver.
    if (data->recv)
    {
        NDIlib_recv_destroy(data->recv);
        data->recv = nullptr;
    }

    // Reset menu option and user data.
    data->configureSuccess = false;
    data->menu->setOptionText(data->index, data->sourceName + " (Not Configured)");
    data->menu->setOptionCallback(data->index, setupCheatingWorker, userData);
}

void lineageCheating()
{
    // TODO: input the config file path.
    CheatingConfig cheatingCfg;
    DebugModeConfig debugModeCfg;
    debugModeCfg.showWindow = true;
    debugModeCfg.windowName = "dev";

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
        data.cheatingCfg = cheatingCfg;
        data.debugModeCfg = debugModeCfg;
        data.worker = nullptr;
        data.configureSuccess = false;
        data.recv = nullptr;
        data.hid = nullptr;
        data.menu = &menu;

        userDatas.emplace_back(data);
        menu.addOption(sourceName + " (Not Configured)", setupCheatingWorker, &userDatas.back());
    }

    menu.addOption("Exit",
        [](void* menu) { static_cast<CommandLineMenu*>(menu)->endReceiveInput(); },
        &menu, false, false);

    menu.show();
    menu.startReceiveInput();

    // Cleanup
    for (auto& data : userDatas)
        cleanupCheatingWorker(&data);

    NDIlib_find_destroy(ndiFinder);
    NDIlib_destroy();
}
