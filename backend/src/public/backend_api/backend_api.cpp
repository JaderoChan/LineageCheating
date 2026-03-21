#include "backend_api.hpp"

#include <algorithm>
#include <chrono>

#include <opencv2/opencv.hpp>

#include <private/game_frame_utils.hpp>
#include <private/identifyHpMp.hpp>
#include <private/opencv_utils.hpp>
#include <private/text_detect_mser.hpp>
#include <private/ocr.hpp>

#include "hid_api.hpp"

CheatingWorker::CheatingWorker(NDIlib_recv_instance_t recv, hid::HID hid,
    const CheatingConfig& cheatingConfig, const DebugModeConfig& debugConfig)
    : recv_(recv), hid_(hid), cheatingConfig_(cheatingConfig), debugModeConfig_(debugConfig)
{}

CheatingWorker::~CheatingWorker()
{
    stop();
}

void CheatingWorker::run()
{
    std::lock_guard<std::mutex> locker(runStopMtx_);

    if (isRunning_.load())
        return;

    shouldClose_.store(false);
    isRunning_.store(true);

    workerThread_ = std::thread([this]() { work(); });
}

void CheatingWorker::stop()
{
    std::lock_guard<std::mutex> locker(runStopMtx_);

    shouldClose_.store(true);

    if (workerThread_.joinable())
        workerThread_.join();

    isRunning_.store(false);
}

bool CheatingWorker::isRunning() const
{
    return isRunning_.load();
}

Progress CheatingWorker::getHp() const
{
    return hp_.load();
}

Progress CheatingWorker::getMp() const
{
    return mp_.load();
}

int CheatingWorker::getArrowCount() const
{
    return arrowNum_.load();
}

void CheatingWorker::work()
{
    bool isWindowCreated = false;
    const CheatingConfig& cheatingCfg = cheatingConfig_;
    const DebugModeConfig& debugCfg = debugModeConfig_;
    GameFrameUtils gameFrameUtils(cheatingCfg);

    NDIlib_framesync_instance_t frameSync = NDIlib_framesync_create(recv_);

    cv::Mat prevFrame, currFrame;
    while (!shouldClose_.load())
    {
        NDIlib_video_frame_v2_t videoFrame;
        NDIlib_framesync_capture_video(frameSync, &videoFrame);

        if (videoFrame.p_data)
        {
            cv::Mat frame(videoFrame.yres, videoFrame.xres, CV_8UC4,
                videoFrame.p_data, videoFrame.line_stride_in_bytes);
            if (debugModeConfig_.showWindow)
            {
                isWindowCreated = true;
                cv::imshow(debugModeConfig_.windowName, frame);
                cv::waitKey(1);
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(33)); // 30 FPS
        }

        NDIlib_framesync_free_video(frameSync, &videoFrame);
    }

    NDIlib_framesync_destroy(frameSync);

    if (debugModeConfig_.showWindow && isWindowCreated)
        cv::destroyWindow(debugModeConfig_.windowName);
}
