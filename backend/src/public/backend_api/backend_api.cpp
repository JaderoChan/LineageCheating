#include "backend_api.hpp"

#include <algorithm>
#include <chrono>

#include <opencv2/opencv.hpp>

#include "hid_api.hpp"

#pragma once

#include <string>
#include <sstream>

inline std::string formatString(const std::string& fmtStr)
{
    return fmtStr;
}

template <typename T>
std::string formatString(const std::string& fmtStr, const T& arg)
{
    std::stringstream ss;

    if (fmtStr.size() < 4)
    {
        size_t pos = fmtStr.find("{}");
        if (pos == std::string::npos)
            return fmtStr;

        ss << fmtStr.substr(0, pos);
        ss << arg;

        return ss.str() + fmtStr.substr(pos + 2);
    }

    std::string window(4, '\0');
    for (size_t i = 0; i < fmtStr.size();)
    {
        window[0] = fmtStr[i];
        window[1] = i < fmtStr.size() - 1 ? fmtStr[i + 1] : '\0';
        window[2] = i < fmtStr.size() - 2 ? fmtStr[i + 2] : '\0';
        window[3] = i < fmtStr.size() - 3 ? fmtStr[i + 3] : '\0';

        if (window == "{{}}")
        {
            ss << "{}";
            i += 4;
            continue;
        }

        if (window[0] == '{' && window[1] == '}')
        {
            ss << arg;
            return ss.str() + fmtStr.substr(i + 2);
        }
        else
        {
            ss << window[0];
            i += 1;
            continue;
        }
    }

    return ss.str();
}

template <typename T, typename... Args>
std::string formatString(const std::string& fmtStr, const T& arg, Args&&... args)
{
    std::stringstream ss;

    if (fmtStr.size() < 4)
    {
        size_t pos = fmtStr.find("{}");
        if (pos == std::string::npos)
            return fmtStr;

        ss << fmtStr.substr(0, pos);
        ss << arg;

        return ss.str() + fmtStr.substr(pos + 2);
    }

    std::string window(4, '\0');
    for (size_t i = 0; i < fmtStr.size();)
    {
        window[0] = fmtStr[i];
        window[1] = i < fmtStr.size() - 1 ? fmtStr[i + 1] : '\0';
        window[2] = i < fmtStr.size() - 2 ? fmtStr[i + 2] : '\0';
        window[3] = i < fmtStr.size() - 3 ? fmtStr[i + 3] : '\0';

        if (window == "{{}}")
        {
            ss << "{}";
            i += 4;
            continue;
        }

        if (window[0] == '{' && window[1] == '}')
        {
            ss << arg;
            return ss.str() + formatString(fmtStr.substr(i + 2), std::forward<Args>(args)...);
        }
        else
        {
            ss << window[0];
            i += 1;
            continue;
        }
    }

    return ss.str();
}

template <typename T>
constexpr T square(const T& x) { return x * x; }

constexpr int MAX_SQUARE = square<int>(UINT8_MAX);

constexpr double computeColorSimilarity(const cv::Vec3b& lhs, const cv::Vec3b& rhs,
    double rWeight = 1.0, double gWeight = 1.0, double bWeight = 1.0)
{
    return 1.0 - (
        square(lhs[0] - rhs[0]) * bWeight +
        square(lhs[1] - rhs[1]) * gWeight +
        square(lhs[2] - rhs[2]) * rWeight) /
        (rWeight + gWeight + bWeight) / MAX_SQUARE;
}

static cv::Mat limitImageSize(const cv::Mat& src, int maxX, int maxY)
{
    assert(maxX > 0 && maxY > 0);

    if ((src.cols <= maxX && src.rows <= maxY) || (src.cols == 0 || src.rows == 0))
        return src.clone();

    double xRatio = static_cast<double>(maxX) / src.cols;
    double yRatio = static_cast<double>(maxY) / src.rows;
    double ratio = std::min(xRatio, yRatio);

    int newCols = src.cols * ratio;
    int newRows = src.rows * ratio;
    if (newCols <= 0 || newRows <= 0)
        throw std::runtime_error("Failed to limit image size");

    cv::Mat limitedImg;
    cv::resize(src, limitedImg, cv::Size(newCols, newRows));

    return limitedImg;
}

CheatingWorker::CheatingWorker(NDIlib_recv_instance_t majorRecv, NDIlib_recv_instance_t minorRecv, hid::HID hid,
    const CheatingConfig& cheatingConfig)
    : majorRecv_(majorRecv), minorecv_(minorRecv), hid_(hid), cheatingConfig_(cheatingConfig)
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

    cv::Vec3b hpThresholdColor(
        cheatingConfig_.hpThresoldColor.b,
        cheatingConfig_.hpThresoldColor.g,
        cheatingConfig_.hpThresoldColor.r);

    CheatingConfig& cheatingCfg = cheatingConfig_;

    milliseconds frameTimeInterval(1000 / cheatingCfg.fps);
    milliseconds clickTimeInterval(1000 / cheatingCfg.cps);
    milliseconds heartTimeInterval(cheatingCfg.heartTimeInterval);

    auto getNewValidFrame = [this](NDIlib_recv_instance_t recv, int timeoutMs = 500) -> cv::Mat
    {
        NDIlib_video_frame_v2_t videoFrame;
        while (!shouldClose_.load())
        {
            auto frameType = NDIlib_recv_capture_v3(recv, &videoFrame, nullptr, nullptr, timeoutMs);
            if (frameType == NDIlib_frame_type_video && videoFrame.p_data)
            {
                cv::Mat frame = cv::Mat(
                    videoFrame.yres, videoFrame.xres, CV_8UC4,
                    videoFrame.p_data, videoFrame.line_stride_in_bytes).clone();
                NDIlib_recv_free_video_v2(recv, &videoFrame);
                cv::cvtColor(frame, frame, cv::COLOR_BGRA2BGR);
                return frame;
            }

            if (frameType == NDIlib_frame_type_video)
                NDIlib_recv_free_video_v2(recv, &videoFrame);
        }
        return cv::Mat();
    };

    auto lastClickTime = high_resolution_clock::now();
    auto clickLeftButton = [&]()
    {
        auto interval = high_resolution_clock::now() - lastClickTime;
        if (interval < clickTimeInterval)
            std::this_thread::sleep_for(clickTimeInterval - interval);
        lastClickTime = high_resolution_clock::now();
        hid::clickMouseButton(hid_, 1);
    };

    auto lastMajorHeartTime = high_resolution_clock::now();
    auto lastMinorHeartTime = high_resolution_clock::now();
    while (!shouldClose_.load())
    {
        cv::Mat majorFrame = getNewValidFrame(majorRecv_);
        cv::Mat minorFrame = getNewValidFrame(minorecv_);

        if (!majorFrame.empty())
        {
            int row = majorFrame.rows * cheatingCfg.hpFlagPointY;
            int col = majorFrame.cols * cheatingCfg.hpFlagPointX;
            auto majorColor = majorFrame.at<cv::Vec3b>(row, col);
            auto similarity = computeColorSimilarity(majorColor, hpThresholdColor);
            if (similarity >= 0.9)
            {
                lastMajorHeartTime = high_resolution_clock::now();
                hid::clickKey(hid_, VK_F6);
            }
        }

        if (!minorFrame.empty())
        {
            int row = minorFrame.rows * cheatingCfg.hpFlagPointY;
            int col = minorFrame.cols * cheatingCfg.hpFlagPointX;
            auto minorColor = minorFrame.at<cv::Vec3b>(row, col);
            auto similarity = computeColorSimilarity(minorColor, hpThresholdColor);
            if (similarity >= 0.9)
            {
                lastMinorHeartTime = high_resolution_clock::now();
                hid::clickKey(hid_, VK_F5);
            }

//             cv::line(minorFrame, cv::Point(0, row), cv::Point(minorFrame.cols, row), cv::Scalar(0, 0, 255), 3);
//             cv::line(minorFrame, cv::Point(col, 0), cv::Point(col, minorFrame.rows), cv::Scalar(0, 0, 255), 3);
//             cv::circle(minorFrame, cv::Point(col, row), 10, cv::Scalar(0, 255, 0), 5);
//             minorFrame =  limitImageSize(minorFrame, 1080, 1080);
//             std::string text = formatString("Pos: [{}, {}], ({}, {})", col, row, cheatingCfg.hpFlagPointX, cheatingCfg.hpFlagPointY);
//             cv::putText(minorFrame, text, cv::Point(5, 60), cv::HersheyFonts::FONT_HERSHEY_COMPLEX, 1, cv::Scalar(255, 0, 0));
//
//             cv::imshow("Minor", minorFrame);
//             int key = cv::waitKey(1);
//             switch (key)
//             {
//                 case 'w':
//                 case 'i':
//                     cheatingCfg.hpFlagPointY = std::clamp(
//                         cheatingCfg.hpFlagPointY - (key == 'w' ? 0.01 : 0.1), 0.0, 1.0);
//                     break;
//                 case 'a':
//                 case 'j':
//                     cheatingCfg.hpFlagPointX = std::clamp(
//                         cheatingCfg.hpFlagPointX - (key == 'a' ? 0.01 : 0.1), 0.0, 1.0);
//                     break;
//                 case 's':
//                 case 'k':
//                     cheatingCfg.hpFlagPointY = std::clamp(
//                         cheatingCfg.hpFlagPointY + (key == 's' ? 0.01 : 0.1), 0.0, 1.0);
//                     break;
//                 case 'd':
//                 case 'l':
//                     cheatingCfg.hpFlagPointX = std::clamp(
//                         cheatingCfg.hpFlagPointX + (key == 'd' ? 0.01 : 0.1), 0.0, 1.0);
//                     break;
//                 default:
//                     break;
//             }
        }

        clickLeftButton();
    }
}
