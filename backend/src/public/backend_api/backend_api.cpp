#include "backend_api.hpp"

#include <algorithm>
#include <chrono>

#include <opencv2/opencv.hpp>

#include <private/game_frame_utils.hpp>
#include <private/identify_hp_mp.hpp>
#include <private/identify_num.hpp>
#include <private/opencv_utils.hpp>
#include <private/ocr.hpp>
#include <private/monster_name.hpp>

#include "hid_api.hpp"

CheatingWorker::CheatingWorker(NDIlib_recv_instance_t majorRecv, NDIlib_recv_instance_t minorRecv, hid::HID hid,
    const CheatingConfig& cheatingConfig, const DebugModeConfig& debugConfig)
    : majorRecv_(majorRecv), minorecv_(minorRecv), hid_(hid),
    cheatingConfig_(cheatingConfig), debugModeConfig_(debugConfig)
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

void CheatingWorker::work()
{
    using namespace std::chrono;

    Progress majorHp;
    Progress majorMp;
    Progress minorHp;
    Progress minorMp;

    bool isWindowCreated = false;
    const CheatingConfig& cheatingCfg = cheatingConfig_;
    const DebugModeConfig& debugCfg = debugModeConfig_;
    GameFrameUtils gameFrameUtils(cheatingCfg);

    milliseconds frameTimeInterval(1000 / cheatingCfg.fps);

    NDIlib_framesync_instance_t majorFrameSync = NDIlib_framesync_create(majorRecv_);
    NDIlib_framesync_instance_t minorFrameSync = NDIlib_framesync_create(minorecv_);
    auto lastGetFrameTime = high_resolution_clock::now();
    auto getNewValidFrame = [=, &lastGetFrameTime](NDIlib_framesync_instance_t frameSync) -> cv::Mat
    {
        NDIlib_video_frame_v2_t videoFrame;
        while (!shouldClose_.load())
        {
            NDIlib_framesync_capture_video(frameSync, &videoFrame);
            if (videoFrame.p_data)
            {
                // Assert video frame FourCC is BGRA.
                cv::Mat frame = cv::Mat(
                    videoFrame.yres, videoFrame.xres, CV_8UC4,
                    videoFrame.p_data, videoFrame.line_stride_in_bytes).clone();
                NDIlib_framesync_free_video(frameSync, &videoFrame);
                cv::cvtColor(frame, frame, cv::COLOR_BGRA2BGR);

                auto interval = high_resolution_clock::now() - lastGetFrameTime;
                if (interval < frameTimeInterval)
                    std::this_thread::sleep_for(frameTimeInterval - interval);
                lastGetFrameTime =  high_resolution_clock::now();
                return frame;
            }
            NDIlib_framesync_free_video(frameSync, &videoFrame);
            std::this_thread::sleep_for(milliseconds(1));
        }
        return cv::Mat();
    };

    auto showDebugFrame = [&](cv::Mat displayFrame)
    {
        if (!debugCfg.showWindow || displayFrame.empty())
            return;

        isWindowCreated = true;
        if (debugCfg.showDebugInfo)
        {
            cv::putText(displayFrame, "Major HP: " + majorHp.toString(),
                cv::Point(5, 60), cv::FONT_HERSHEY_SIMPLEX, 2.0, cv::Scalar(255, 0, 0), 3);
            cv::putText(displayFrame, "Major MP: " + majorMp.toString(),
                cv::Point(5, 120), cv::FONT_HERSHEY_SIMPLEX, 2.0, cv::Scalar(255, 0, 0), 3);
            cv::putText(displayFrame, "Minor HP: " + minorHp.toString(),
                cv::Point(5, 60), cv::FONT_HERSHEY_SIMPLEX, 2.0, cv::Scalar(255, 0, 0), 3);
            cv::putText(displayFrame, "Minor MP: " + minorMp.toString(),
                cv::Point(5, 120), cv::FONT_HERSHEY_SIMPLEX, 2.0, cv::Scalar(255, 0, 0), 3);
        }

        if (debugCfg.windowMaxX <= 0 || debugCfg.windowMaxY <= 0)
            cv::imshow(debugCfg.windowName, displayFrame);
        else
            cv::imshow(debugCfg.windowName, limitImageSize(displayFrame, debugCfg.windowMaxX, debugCfg.windowMaxY));
        cv::waitKey(1);
    };

    while (!shouldClose_.load())
    {
        cv::Mat majorFrame = getNewValidFrame(majorFrameSync);
        cv::Mat minorFrame = getNewValidFrame(minorFrameSync);

        // Identify HP and MP.
        HpMp majorHpmp = identifyHpMp(gameFrameUtils.getHpMpAreaFrame(majorFrame));
        majorHp = majorHpmp.hp;
        majorMp = majorHpmp.mp;

        HpMp minorHpmp = identifyHpMp(gameFrameUtils.getHpMpAreaFrame(minorFrame));
        minorHp = minorHpmp.hp;
        minorMp = minorHpmp.mp;

        showDebugFrame(majorFrame);

        // If the HP is low, press F5 to use potion.
        if (majorHp.isValid() && majorHp.getPercentage() < cheatingCfg.hpThresholdPercent)
            hid::clickKey(hid_, VK_F7);
        if (minorHp.isValid() && minorHp.getPercentage() < cheatingCfg.hpThresholdPercent)
            hid::clickKey(hid_, VK_F8);
    }

    NDIlib_framesync_destroy(majorFrameSync);
    NDIlib_framesync_destroy(minorFrameSync);

    if (debugModeConfig_.showWindow && isWindowCreated)
        cv::destroyWindow(debugModeConfig_.windowName);
}
