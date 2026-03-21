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
    using namespace std::chrono;

    bool isWindowCreated = false;
    const CheatingConfig& cheatingCfg = cheatingConfig_;
    const DebugModeConfig& debugCfg = debugModeConfig_;
    GameFrameUtils gameFrameUtils(cheatingCfg);

    NDIlib_framesync_instance_t frameSync = NDIlib_framesync_create(recv_);

    // Main game area frame.
    cv::Mat prevFrame, currFrame;
    {
        // Init the prevFrame
        NDIlib_video_frame_v2_t videoFrame;
        while (true)
        {
            NDIlib_framesync_capture_video(frameSync, &videoFrame);
            if (videoFrame.p_data)
            {
                prevFrame = cv::Mat(
                    videoFrame.yres, videoFrame.xres, CV_8UC4,
                    videoFrame.p_data, videoFrame.line_stride_in_bytes);
                // Crop frame to only keep main game area.
                prevFrame = gameFrameUtils.getHandledMainGameAreaFrame(prevFrame).clone();
                currFrame = prevFrame.clone();
                NDIlib_framesync_free_video(frameSync, &videoFrame);
                break;
            }
            NDIlib_framesync_free_video(frameSync, &videoFrame);
        }
    }

    while (!shouldClose_.load())
    {
        NDIlib_video_frame_v2_t videoFrame;
        NDIlib_framesync_capture_video(frameSync, &videoFrame);

        if (videoFrame.p_data)
        {
            cv::Mat frame = cv::Mat(videoFrame.yres, videoFrame.xres, CV_8UC4,
                videoFrame.p_data, videoFrame.line_stride_in_bytes).clone();

            // Main cheating code

            prevFrame = currFrame;
            currFrame = gameFrameUtils.getHandledMainGameAreaFrame(frame).clone();

            // Identify HP and MP.
            hp_.store(identifyHpMp(gameFrameUtils.getHpAreaFrame(frame)));
            mp_.store(identifyHpMp(gameFrameUtils.getMpAreaFrame(frame)));

            bool gotTarget = false;
            // The difference area between two frames.
            std::vector<cv::Rect> diffRects = getImageDiffs(prevFrame, currFrame);
            while (!diffRects.empty() && !gotTarget)
            {
                // Map the cropped image back to the original image coordinate system.
                auto rect = gameFrameUtils.mapMainGameAreaRectToSource(frame, diffRects.back());

                if (debugModeConfig_.showDiffRect)
                    cv::rectangle(frame, rect, cv::Scalar(0, 255, 0));

                // Move the mouse to the place where the frame changes.
                cv::Point centerPt = (rect.tl() + rect.br()) / 2;
                hid::moveMouseTo(hid_, centerPt.x, centerPt.y);
                std::this_thread::sleep_for(microseconds(cheatingCfg.sleepAfterMove));

                // Expand text recognition range.
                rect.x -= cheatingCfg.textRegionExpansionX;
                rect.y -= cheatingCfg.textRegionExpansionY;
                rect.width += cheatingCfg.textRegionExpansionX * 2;
                rect.height += cheatingCfg.textRegionExpansionY * 2;

                // Get new frame

                diffRects.pop_back();
            }

            // Debug mode info display.
            if (debugModeConfig_.showWindow)
            {
                isWindowCreated = true;
                if (debugModeConfig_.showHpMp)
                {
                    cv::putText(frame, "HP: " + hp_.load().toString(),
                        cv::Point(5, 20), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 0, 0));
                    cv::putText(frame, "MP: " + mp_.load().toString(),
                        cv::Point(5, 40), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 0, 0));
                }
                cv::imshow(debugModeConfig_.windowName, frame);
                cv::waitKey(1);
            }

            std::this_thread::sleep_for(milliseconds(33)); // 30 FPS
        }

        NDIlib_framesync_free_video(frameSync, &videoFrame);
    }

    NDIlib_framesync_destroy(frameSync);

    if (debugModeConfig_.showWindow && isWindowCreated)
        cv::destroyWindow(debugModeConfig_.windowName);
}
