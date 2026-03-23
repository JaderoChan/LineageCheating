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

void CheatingWorker::work()
{
    using namespace std::chrono;

    Progress hp;
    Progress mp;
    int arrowNum = -1;

    bool isWindowCreated = false;
    const CheatingConfig& cheatingCfg = cheatingConfig_;
    const DebugModeConfig& debugCfg = debugModeConfig_;
    GameFrameUtils gameFrameUtils(cheatingCfg);

    NDIlib_framesync_instance_t frameSync = NDIlib_framesync_create(recv_);
    auto getNewValidFrame = [frameSync, this]() -> cv::Mat
    {
        NDIlib_video_frame_v2_t videoFrame;
        while (!shouldClose_.load())
        {
            NDIlib_framesync_capture_video(frameSync, &videoFrame);
            if (videoFrame.p_data)
            {
                cv::Mat frame = cv::Mat(
                    videoFrame.yres, videoFrame.xres, CV_8UC4,
                    videoFrame.p_data, videoFrame.line_stride_in_bytes).clone();
                NDIlib_framesync_free_video(frameSync, &videoFrame);
                cv::cvtColor(frame, frame, cv::COLOR_BGRA2BGR);
                return frame;
            }
            NDIlib_framesync_free_video(frameSync, &videoFrame);
        }
        return cv::Mat();
    };

    auto showDebugFrame = [&](const cv::Mat& displayFrame, const std::string& attackTarget = std::string())
    {
        if (!debugCfg.showWindow || displayFrame.empty())
            return;

        isWindowCreated = true;
        cv::Mat showFrame = displayFrame.clone();

        if (debugCfg.showDebugInfo)
        {
            cv::putText(showFrame, "HP: " + hp.toString(),
                cv::Point(5, 20), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 0, 0));
            cv::putText(showFrame, "MP: " + mp.toString(),
                cv::Point(5, 40), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 0, 0));
            cv::putText(showFrame, "Arrow Num: " + std::to_string(arrowNum),
                cv::Point(5, 60), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 0, 0));
            cv::putText(showFrame, "Attack Target: " + attackTarget.empty() ? "-" : attackTarget,
                cv::Point(5, 80), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 0, 0));
        }

        cv::imshow(debugCfg.windowName, showFrame);
        cv::waitKey(1);
    };

    // Main game area frame.
    cv::Mat prevFrame, currFrame;
    currFrame = gameFrameUtils.getMainGameAreaFrame(getNewValidFrame());

    while (!shouldClose_.load())
    {
        cv::Mat frame = getNewValidFrame();
        prevFrame = currFrame.clone();
        currFrame = gameFrameUtils.getMainGameAreaFrame(frame);

        // Identify HP and MP.
        hp = identifyHpMp(gameFrameUtils.getHpAreaFrame(frame));
        mp = identifyHpMp(gameFrameUtils.getMpAreaFrame(frame));
        printf("HP: %s\n", hp.toString().c_str(), mp.toString().c_str());

        showDebugFrame(frame);

        // If the HP is low, press F5 to use potion.
        if (hp.isValid() && hp.getPercentage() < cheatingCfg.hpThresholdPercent)
            hid::clickKey(hid_, VK_F5);

        // The difference area between two frames.
        std::vector<cv::Rect> diffRects = getImageDiffs(prevFrame, currFrame);
        if (!diffRects.empty())
            diffRects.resize(5);

        bool gotTarget = false;
        while (!diffRects.empty() && !gotTarget)
        {
            printf("Has diff rect\n");
            // Map the cropped image back to the original image coordinate system.
            auto textRect = gameFrameUtils.mapMainGameAreaRectToSource(frame, diffRects.back());
            if (debugModeConfig_.showWindow && debugModeConfig_.showDiffRect)
                cv::rectangle(frame, textRect, cv::Scalar(0, 255, 0));

            // Move the mouse to the place where the frame changes.
            cv::Point centerPt = (textRect.tl() + textRect.br()) / 2;
            hid::moveMouseTo(hid_, centerPt.x, centerPt.y);
            std::this_thread::sleep_for(microseconds(cheatingCfg.sleepAfterMove));

            // Expand text recognition range.
            textRect.x -= cheatingCfg.textRegionExpansionX;
            textRect.y -= cheatingCfg.textRegionExpansionY;
            textRect.width += cheatingCfg.textRegionExpansionX * 2;
            textRect.height += cheatingCfg.textRegionExpansionY * 2;

            // Clamp the expanded rect to frame boundaries.
            textRect &= cv::Rect(0, 0, frame.cols, frame.rows);

            frame = getNewValidFrame();

            auto& ocrLite = getOcrLiteInstance();
            auto result = ocrLite.detect(frame(textRect), 50, 1024, 0.6, 0.3, 1.5, false, false);

            for (const auto& box : result.textBlocks)
            {
                printf("Detext Text: %s\n", box.text.c_str());
                if (isMonsterName(box.text))
                {
                    printf("Target Name: %s\n", box.text.c_str());
                    // Press shift
                    hid::pressKey(hid_, VK_LSHIFT);
                    // Press left mouse button (hold down) to start attacking.
                    hid::pressMouseButton(hid_, 1);

                    // Monitor arrow count to detect monster death.
                    int lastArrowNum = -1;
                    int arrowNum = -1;
                    auto lastArrowChangeTime = steady_clock::now();

                    while (!shouldClose_.load())
                    {
                        frame = getNewValidFrame();
                        prevFrame = currFrame.clone();
                        currFrame = gameFrameUtils.getMainGameAreaFrame(frame);

                        // Identify HP and MP.
                        hp = identifyHpMp(gameFrameUtils.getHpAreaFrame(frame));
                        mp = identifyHpMp(gameFrameUtils.getMpAreaFrame(frame));
                        showDebugFrame(frame);

                        // If the HP is low, press F5 to use potion.
                        if (hp.isValid() && hp.getPercentage() < cheatingCfg.hpThresholdPercent)
                            hid::clickKey(hid_, VK_F5);

                        // Get current arrow count.
                        arrowNum = identifyNum(gameFrameUtils.getArrowAreaFrame(frame));

                        if (arrowNum != lastArrowNum)
                        {
                            // Arrow count changed, monster is still alive.
                            lastArrowNum = arrowNum;
                            lastArrowChangeTime = steady_clock::now();
                        }
                        else
                        {
                            // Arrow count unchanged, check if threshold exceeded.
                            auto elapsed = duration_cast<milliseconds>(
                                steady_clock::now() - lastArrowChangeTime).count();
                            if (elapsed >= cheatingCfg.arrowUnchangedTimeThreshold)
                                break;
                        }

                        showDebugFrame(frame);;
                    }

                    // Release left mouse button.
                    hid::releaseMouseButton(hid_, 1);
                    hid::releaseKey(hid_, VK_LSHIFT);

                    // Update prevFrame and currFrame after combat for next iteration.
                    // Break out of diffRects loop to start fresh scanning.
                    gotTarget = true;
                    break;
                }
            }

            if (gotTarget)
                break;
            diffRects.pop_back();
        }
    }

    NDIlib_framesync_destroy(frameSync);

    if (debugModeConfig_.showWindow && isWindowCreated)
        cv::destroyWindow(debugModeConfig_.windowName);
}
