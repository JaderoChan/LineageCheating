#pragma once

#include <atomic>
#include <mutex>
#include <thread>

#include <Processing.NDI.Lib.h>

#include "hid_api.hpp"
#include "cheating_config.hpp"
#include "progress.hpp"

class CheatingWorker
{
public:
    CheatingWorker(NDIlib_recv_instance_t recv, hid::HID hid,
        const CheatingConfig& cheatingConfig, const DebugModeConfig& debugConfig);
    ~CheatingWorker();

    void run();
    void stop();

    bool isRunning() const;

    Progress getHp() const;
    Progress getMp() const;
    int getArrowCount() const;

private:
    void work();

    std::thread workerThread_;
    mutable std::mutex runStopMtx_;
    std::atomic<bool> shouldClose_{false};
    std::atomic<bool> isRunning_{false};

    DebugModeConfig debugModeConfig_;
    CheatingConfig cheatingConfig_;

    NDIlib_recv_instance_t recv_;
    hid::HID hid_;

    std::atomic<Progress> hp_;
    std::atomic<Progress> mp_;
    std::atomic<int> arrowNum_{0};
};
