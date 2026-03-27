#pragma once

#include <atomic>
#include <mutex>
#include <thread>

#include <Processing.NDI.Lib.h>

#include "hid_api.hpp"
#include "cheating_config.hpp"

// Major: Knight/Main player. Minor: Assist player.
class CheatingWorker
{
public:
    CheatingWorker(NDIlib_recv_instance_t majorRecv, NDIlib_recv_instance_t minorRecv, hid::HID hid,
        const CheatingConfig& cheatingConfig);
    ~CheatingWorker();

    void run();
    void stop();

    bool isRunning() const;

private:
    void work1();
    void work2();

    std::thread workerThread1_;
    std::thread workerThread2_;
    mutable std::mutex runStopMtx_;
    std::atomic<bool> shouldClose_{false};
    std::atomic<bool> isRunning_{false};

    CheatingConfig cheatingConfig_;

    NDIlib_recv_instance_t majorRecv_;
    NDIlib_recv_instance_t minorRecv_;
    hid::HID hid_;
};
