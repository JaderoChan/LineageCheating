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
    std::string attackTarget;

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
                // Assert video frame FourCC is BGRA.
                cv::Mat frame = cv::Mat(
                    videoFrame.yres, videoFrame.xres, CV_8UC4,
                    videoFrame.p_data, videoFrame.line_stride_in_bytes).clone();
                NDIlib_framesync_free_video(frameSync, &videoFrame);
                cv::cvtColor(frame, frame, cv::COLOR_BGRA2BGR);
                printf("Get new frame\n");
                return frame;
            }
            NDIlib_framesync_free_video(frameSync, &videoFrame);
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
            cv::putText(displayFrame, "HP: " + hp.toString(),
                cv::Point(5, 60), cv::FONT_HERSHEY_SIMPLEX, 2.0, cv::Scalar(255, 0, 0), 3);
            cv::putText(displayFrame, "MP: " + mp.toString(),
                cv::Point(5, 120), cv::FONT_HERSHEY_SIMPLEX, 2.0, cv::Scalar(255, 0, 0), 3);
            cv::putText(displayFrame, "Arrow Num: " + std::to_string(arrowNum),
                cv::Point(5, 180), cv::FONT_HERSHEY_SIMPLEX, 2.0, cv::Scalar(255, 0, 0), 3);
            cv::putText(displayFrame, "Attack Target: " + (attackTarget.empty() ? "-" : attackTarget),
                cv::Point(5, 240), cv::FONT_HERSHEY_SIMPLEX, 2.0, cv::Scalar(255, 0, 0), 3);
        }

        if (debugCfg.windowMaxX <= 0 || debugCfg.windowMaxY <= 0)
            cv::imshow(debugCfg.windowName, displayFrame);
        else
            cv::imshow(debugCfg.windowName, limitImageSize(displayFrame, debugCfg.windowMaxX, debugCfg.windowMaxY));
        cv::waitKey(1);
    };

    // Main game area frame.
    cv::Mat prevFrame, currFrame;
    currFrame = gameFrameUtils.getMainGameAreaFrame(getNewValidFrame());

    while (!shouldClose_.load())
    {
        attackTarget.clear();

        cv::Mat frame = getNewValidFrame();
        prevFrame = currFrame.clone();
        currFrame = gameFrameUtils.getMainGameAreaFrame(frame);

        // Identify HP and MP.
        hp = identifyHpMp(gameFrameUtils.getHpAreaFrame(frame));
        mp = identifyHpMp(gameFrameUtils.getMpAreaFrame(frame));

        showDebugFrame(frame);

        // If the HP is low, press F5 to use potion.
        if (hp.isValid() && hp.getPercentage() < cheatingCfg.hpThresholdPercent)
            hid::clickKey(hid_, VK_F5);

        // The difference area between two frames.
        std::vector<cv::Rect> diffRects = getImageDiffs(prevFrame, currFrame);
        if (!diffRects.empty())
            diffRects.resize(10);

        if (debugModeConfig_.showWindow && debugModeConfig_.showDiffRect)
        {
            cv::Mat drawedFrame = frame.clone();
            for (const auto& rect : diffRects)
            {
                auto srcRect = gameFrameUtils.mapMainGameAreaRectToSource(frame, rect);
                cv::rectangle(drawedFrame, srcRect, cv::Scalar(0, 0, 255), 3);
            }
            showDebugFrame(drawedFrame);
        }

        bool gotTarget = false;
        while (!diffRects.empty() && !gotTarget && !shouldClose_.load())
        {
            // Map the cropped image back to the original image coordinate system.
            auto diffRect = gameFrameUtils.mapMainGameAreaRectToSource(frame, diffRects.back());

            // Move the mouse to the place where the frame changes.
            cv::Point centerPt = (diffRect.tl() + diffRect.br()) / 2;
            hid::moveMouseTo(hid_, centerPt.x, centerPt.y);
            std::this_thread::sleep_for(microseconds(cheatingCfg.sleepAfterMove));

            // Expand text recognition range.
            diffRect.x -= cheatingCfg.textRegionExpansionX;
            diffRect.y -= cheatingCfg.textRegionExpansionY;
            diffRect.width += cheatingCfg.textRegionExpansionX * 2;
            diffRect.height += cheatingCfg.textRegionExpansionY * 2;

            // Clamp the expanded rect to frame boundaries.
            diffRect &= cv::Rect(0, 0, frame.cols, frame.rows);

            frame = getNewValidFrame();
            showDebugFrame(frame);

            auto& ocrLite = getOcrLiteInstance();
            auto result = ocrLite.detect(frame(diffRect), 50, 1024, 0.35, 0.3, 1.6, false, false);

            for (const auto& box : result.textBlocks)
            {
                printf("Detext Text: %s\n", box.text.c_str());
                if (debugCfg.showWindow && debugCfg.showTextRect)
                {
                    cv::Rect rect = cv::boundingRect(box.boxPoint);
                    rect = cv::Rect(diffRect.x + rect.x, diffRect.y + rect.y, rect.width, rect.height);

                    cv::Mat drawedFrame = frame.clone();
                    cv::rectangle(drawedFrame, rect, cv::Scalar(0, 255, 0), 3);
                    showDebugFrame(drawedFrame);
                }

                if (isMonsterName(box.text))
                {
                    printf("Is Monster name: %s\n", box.text.c_str());
                    attackTarget = box.text;

                    // Press shift
                    hid::pressKey(hid_, VK_LSHIFT);
                    // Press left mouse button (hold down) to start attacking.
                    hid::pressMouseButton(hid_, 1);

                    // Monitor arrow count to detect monster death.
                    int lastArrowNum = -1;
                    auto lastArrowChangeTime = steady_clock::now();

                    while (!shouldClose_.load())
                    {
                        frame = getNewValidFrame();
                        prevFrame = currFrame.clone();
                        currFrame = gameFrameUtils.getMainGameAreaFrame(frame);

                        // Identify HP and MP.
                        hp = identifyHpMp(gameFrameUtils.getHpAreaFrame(frame));
                        mp = identifyHpMp(gameFrameUtils.getMpAreaFrame(frame));

                        // If the HP is low, press F5 to use potion.
                        if (hp.isValid() && hp.getPercentage() < cheatingCfg.hpThresholdPercent)
                            hid::clickKey(hid_, VK_F5);

                        // Get current arrow count.
                        arrowNum = identifyNum(gameFrameUtils.getArrowAreaFrame(frame));
                        showDebugFrame(frame);

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
