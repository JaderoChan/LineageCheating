#pragma once

#include <atomic>
#include <mutex>
#include <thread>

#include <Processing.NDI.Lib.h>

#include "hid_api.hpp"
#include "cheating_config.hpp"
#include "progress.hpp"

// Major: Knight/Main player. Minor: Assist player.
class CheatingWorker
{
public:
    CheatingWorker(NDIlib_recv_instance_t majorRecv, NDIlib_recv_instance_t minorRecv, hid::HID hid,
        const CheatingConfig& cheatingConfig, const DebugModeConfig& debugConfig);
    ~CheatingWorker();

    void run();
    void stop();

    bool isRunning() const;

private:
    void work();

    std::thread workerThread_;
    mutable std::mutex runStopMtx_;
    std::atomic<bool> shouldClose_{false};
    std::atomic<bool> isRunning_{false};

    DebugModeConfig debugModeConfig_;
    CheatingConfig cheatingConfig_;

    NDIlib_recv_instance_t majorRecv_;
    NDIlib_recv_instance_t minorecv_;
    hid::HID hid_;
};
