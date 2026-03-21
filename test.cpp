#include "backend_api.hpp"

#include <algorithm>
#include <chrono>

#include <opencv2/opencv.hpp>

#include <private/game_frame_utils.hpp>
#include <private/identify_hp_mp.hpp>
#include <private/identify_num.hpp>
#include <private/opencv_utils.hpp>
#include <private/text_detect_mser.hpp>
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
            NDIlib_framesync_free_video(frameSync, &videoFrame);

            // Main cheating code

            prevFrame = currFrame;
            currFrame = gameFrameUtils.getHandledMainGameAreaFrame(frame).clone();

            // Identify HP and MP.
            hp_.store(identifyHpMp(gameFrameUtils.getHpAreaFrame(frame)));
            mp_.store(identifyHpMp(gameFrameUtils.getMpAreaFrame(frame)));

            // If the HP is low, press F5 to use potion.
            if (hp_.load().current > 0 && hp_.load().getPercentage() < cheatingCfg.hpThresholdPercent)
                hid::pressKey(hid_, VK_F5);

            // The difference area between two frames.
            std::vector<cv::Rect> diffRects = getImageDiffs(prevFrame, currFrame);
            while (!diffRects.empty())
            {
                bool gotTarget = false;

                // Map the cropped image back to the original image coordinate system.
                auto textRect = gameFrameUtils.mapMainGameAreaRectToSource(frame, diffRects.back());
                if (debugModeConfig_.showDiffRect)
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

                // Perform text recognition on the expanded region.
                if (textRect.width > 0 && textRect.height > 0)
                {
                    cv::Mat textRegion = frame(textRect);

                    // Detect text regions via MSER and then OCR.
                    std::vector<cv::Rect> textBoxes = getTextRegions(textRegion);
                    std::string recognizedText;
                    for (const auto& box : textBoxes)
                    {
                        cv::Rect clampedBox = box & cv::Rect(0, 0, textRegion.cols, textRegion.rows);
                        if (clampedBox.width > 0 && clampedBox.height > 0)
                        {
                            std::string text = getImageText(textRegion(clampedBox));
                            if (!text.empty())
                                recognizedText += text;
                        }
                    }

                    // Check if the recognized text is a monster name.
                    if (isMonsterName(recognizedText))
                    {
                        gotTarget = true;

                        // Press left mouse button (hold down) to start attacking.
                        hid::pressMouseButton(hid_, 1);

                        // Monitor arrow count to detect monster death.
                        int lastArrowCount = -1;
                        auto lastArrowChangeTime = steady_clock::now();

                        while (!shouldClose_.load())
                        {
                            // Capture a new frame to monitor arrow count.
                            NDIlib_video_frame_v2_t attackFrame;
                            NDIlib_framesync_capture_video(frameSync, &attackFrame);

                            if (attackFrame.p_data)
                            {
                                cv::Mat newFrame = cv::Mat(
                                    attackFrame.yres, attackFrame.xres, CV_8UC4,
                                    attackFrame.p_data, attackFrame.line_stride_in_bytes).clone();
                                NDIlib_framesync_free_video(frameSync, &attackFrame);

                                // Update HP/MP during combat.
                                hp = identifyHpMp(gameFrameUtils.getHpAreaFrame(newFrame));
                                mp = identifyHpMp(gameFrameUtils.getMpAreaFrame(newFrame));

                                // Use potion if HP is low during combat.
                                if (hp.current > 0 && hp.getPercentage() < cheatingCfg.hpThresholdPercent)
                                    hid::pressKey(hid_, VK_F5);

                                // Get current arrow count.
                                int currentArrowCount = identifyNum(newFrame);
                                arrowNum = currentArrowCount;

                                if (currentArrowCount != lastArrowCount)
                                {
                                    // Arrow count changed, monster is still alive.
                                    lastArrowCount = currentArrowCount;
                                    lastArrowChangeTime = steady_clock::now();
                                }
                                else
                                {
                                    // Arrow count unchanged, check if threshold exceeded.
                                    auto elapsed = duration_cast<milliseconds>(
                                        steady_clock::now() - lastArrowChangeTime).count();
                                    if (elapsed >= cheatingCfg.arrowUnchangedTimeThreshold)
                                    {
                                        // Monster is likely dead, release mouse and break.
                                        break;
                                    }
                                }

                                // Debug mode info display during combat.
                                if (debugModeConfig_.showWindow)
                                {
                                    isWindowCreated = true;
                                    if (debugModeConfig_.showHpMp)
                                    {
                                        cv::putText(newFrame, "HP: " + hp.toString(),
                                            cv::Point(5, 20), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 0, 0));
                                        cv::putText(newFrame, "MP: " + mp.toString(),
                                            cv::Point(5, 40), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 0, 0));
                                    }
                                    cv::putText(newFrame, "Attacking: " + recognizedText,
                                        cv::Point(5, 60), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 255));
                                    cv::putText(newFrame, "Arrows: " + std::to_string(currentArrowCount),
                                        cv::Point(5, 80), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 255));
                                    cv::imshow(debugModeConfig_.windowName, newFrame);
                                    cv::waitKey(1);
                                }

                                std::this_thread::sleep_for(milliseconds(33));
                            }
                            else
                            {
                                NDIlib_framesync_free_video(frameSync, &attackFrame);
                            }
                        }

                        // Release left mouse button.
                        hid::releaseMouseButton(hid_, 1);

                        // Update prevFrame and currFrame after combat for next iteration.
                        // Break out of diffRects loop to start fresh scanning.
                        break;
                    }
                }

                // Remove the processed diff rect.
                diffRects.pop_back();

                // If we found and attacked a target, break out to next main loop iteration.
                if (gotTarget)
                    break;

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
            }

            std::this_thread::sleep_for(milliseconds(33)); // 30 FPS
        }
        else
        {
            NDIlib_framesync_free_video(frameSync, &videoFrame);
        }
    }

    NDIlib_framesync_destroy(frameSync);

    if (debugModeConfig_.showWindow && isWindowCreated)
        cv::destroyWindow(debugModeConfig_.windowName);
}
