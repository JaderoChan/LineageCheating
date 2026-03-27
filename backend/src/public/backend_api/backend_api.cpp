#include "backend_api.hpp"

#include <algorithm>
#include <chrono>

#include <opencv2/opencv.hpp>

#include "hid_api.hpp"
#include "format_string.hpp"

template <typename T>
constexpr T square(const T& x) { return x * x; }

constexpr int MAX_SQUARE = square<int>(UINT8_MAX);

constexpr double computeColorSimilarity(const cv::Vec3b& lhs, const cv::Vec3b& rhs,
    double rWeight = 1.0, double gWeight = 1.0, double bWeight = 1.0)
{
    return 1.0 - (
        square(static_cast<int>(lhs[0]) - rhs[0]) * bWeight +
        square(static_cast<int>(lhs[1]) - rhs[1]) * gWeight +
        square(static_cast<int>(lhs[2]) - rhs[2]) * rWeight) /
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
    : majorRecv_(majorRecv), minorRecv_(minorRecv), hid_(hid), cheatingConfig_(cheatingConfig)
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

    workerThread1_ = std::thread([this]() { work1(); });
    workerThread2_ = std::thread([this]() { work2(); });
}

void CheatingWorker::stop()
{
    std::lock_guard<std::mutex> locker(runStopMtx_);

    shouldClose_.store(true);

    if (workerThread1_.joinable())
        workerThread1_.join();
    if (workerThread2_.joinable())
        workerThread2_.join();

    isRunning_.store(false);
}

bool CheatingWorker::isRunning() const
{
    return isRunning_.load();
}

void CheatingWorker::work1()
{
    using namespace std::chrono;

    cv::Vec3b hpThresholdColor(
        cheatingConfig_.hpThresoldColor.b,
        cheatingConfig_.hpThresoldColor.g,
        cheatingConfig_.hpThresoldColor.r);

    CheatingConfig cheatingCfg = cheatingConfig_;

    milliseconds heartTimeInterval(cheatingCfg.heartTimeInterval);

    auto getNewValidFrame = [this](NDIlib_recv_instance_t recv, int timeoutMs = 500) -> cv::Mat
    {
        NDIlib_video_frame_v2_t videoFrame;
        bool logFlag = true;
        while (!shouldClose_.load())
        {
            printf("Try Get Frame%s\n", logFlag ? "." : "...");
            logFlag = !logFlag;

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

    size_t frameNum = 0;
    auto lastMajorHeartTime = high_resolution_clock::now();
    auto lastMinorHeartTime = high_resolution_clock::now();
    while (!shouldClose_.load())
    {
        cv::Mat majorFrame = getNewValidFrame(majorRecv_);
        cv::Mat minorFrame = getNewValidFrame(minorRecv_);
        printf("Get frame: %zu\n", frameNum++);

        if (!majorFrame.empty())
        {
            int row = majorFrame.rows * cheatingCfg.hpFlagPointY;
            int col = majorFrame.cols * cheatingCfg.hpFlagPointX;
            auto majorColor = majorFrame.at<cv::Vec3b>(row, col);
            auto similarity = computeColorSimilarity(majorColor, hpThresholdColor);
            if (similarity >= 0.9 && (high_resolution_clock::now() - lastMajorHeartTime >= heartTimeInterval))
            {
                hid::clickKey(hid_, VK_F6);
                lastMajorHeartTime = high_resolution_clock::now();
            }
        }

        if (!minorFrame.empty())
        {
            int row = minorFrame.rows * cheatingCfg.hpFlagPointY;
            int col = minorFrame.cols * cheatingCfg.hpFlagPointX;
            auto minorColor = minorFrame.at<cv::Vec3b>(row, col);
            auto similarity = computeColorSimilarity(minorColor, hpThresholdColor);
            if (similarity >= 0.9 && (high_resolution_clock::now() - lastMinorHeartTime >= heartTimeInterval))
            {
                hid::clickKey(hid_, VK_F5);
                lastMinorHeartTime = high_resolution_clock::now();
            }

//             cv::line(minorFrame, cv::Point(0, row), cv::Point(minorFrame.cols, row), cv::Scalar(0, 0, 255), 3);
//             cv::line(minorFrame, cv::Point(col, 0), cv::Point(col, minorFrame.rows), cv::Scalar(0, 0, 255), 3);
//             cv::circle(minorFrame, cv::Point(col, row), 10, cv::Scalar(0, 255, 0), 5);
//             minorFrame =  limitImageSize(minorFrame, 1080, 1080);
//             std::string text = formatString("Pos: [{}, {}], ({}, {})", col, row, cheatingCfg.hpFlagPointX, cheatingCfg.hpFlagPointY);
//             cv::putText(minorFrame, text, cv::Point(5, 30), cv::HersheyFonts::FONT_HERSHEY_COMPLEX, 1, cv::Scalar(255, 0, 0));
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
    }
}

void CheatingWorker::work2()
{
    using namespace std::chrono;

    CheatingConfig cheatingCfg = cheatingConfig_;

    assert(cheatingCfg.cps != 0);
    milliseconds clickTimeInterval(1000 / cheatingCfg.cps);

    auto lastClickTime = high_resolution_clock::now();
    auto clickLeftButton = [&]()
    {
        auto interval = high_resolution_clock::now() - lastClickTime;
        if (interval < clickTimeInterval)
            std::this_thread::sleep_for(clickTimeInterval - interval);
        lastClickTime = high_resolution_clock::now();
        hid::clickMouseButton(hid_, 1);
    };

    while (!shouldClose_.load())
        clickLeftButton();
}
