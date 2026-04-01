#include "assist_program.hpp"

#include <algorithm>
#include <chrono>

#include "data_struct_converter.hpp"
#include "format_string.hpp"
#include "hid_api.hpp"
#include "opencv_utils.hpp"

static void drawRectOnMat(cv::Mat& mat, const ProportionRect& rect,
    const cv::Scalar& color = cv::Scalar(0, 255, 0), int thickness = 1)
{
    cv::rectangle(mat, convertProportionRectToCvRect(rect, mat.cols, mat.rows), color, thickness);
}

static void showDebugWindow(cv::Mat& frame, const std::string& winName,
    const GameData& gameData, const AssistProgramConfig& config)
{
    // 画框
    drawRectOnMat(frame, gameData.mainRect);
    drawRectOnMat(frame, gameData.playerRect);
    drawRectOnMat(frame, gameData.bottomRect);
    drawRectOnMat(frame, gameData.buffRect);
    drawRectOnMat(frame, gameData.popupMenuRect);
    drawRectOnMat(frame, gameData.hpRect);
    drawRectOnMat(frame, gameData.mpRect);
    drawRectOnMat(frame, gameData.chatRect);

    cv::Mat hpArea = getMatView(frame, gameData.hpRect);
    assert(!hpArea.empty());

    // 高亮当前血条采样点。（治疗阈值）
    auto samplingPt = convertProportionPosToCvPoint(gameData.hpMpBarInfo.samplingPos, hpArea.cols, hpArea.rows);
    cv::circle(hpArea, samplingPt, 4, cv::Scalar(0, 255, 0), cv::FILLED);

    // 高亮当前血条采样点。（回城阈值）
    auto proportionPos = gameData.hpMpBarInfo.samplingPos;
    proportionPos.x = config.backHomeHpThreshold;
    samplingPt = convertProportionPosToCvPoint(proportionPos, hpArea.cols, hpArea.rows);
    cv::circle(hpArea, samplingPt, 4, cv::Scalar(0, 255, 0), cv::FILLED);

    if (config.limitDebugWindowSize)
    {
        assert(config.debugWindowMaxWidth > 0 && config.debugWindowMaxHeight > 0);
        frame = limitImageSize(frame, config.debugWindowMaxWidth, config.debugWindowMaxHeight);
    }

    cv::imshow(winName, frame);
    cv::waitKey(1);
}

AssistProgram::AssistProgram(
    NDIlib_recv_instance_t masterRecv, NDIlib_recv_instance_t footmanRecv, hid::HID footmanHid,
    const GameData& gameData, const AssistProgramConfig& config)
    : masterRecv_(masterRecv), footmanRecv_(footmanRecv), footmanHid_(footmanHid),
    gameData_(gameData), config_(config)
{}

AssistProgram::~AssistProgram()
{
    stop();
}

GameData AssistProgram::getGameData() const
{
    std::lock_guard<std::mutex> locker(gameDataMtx_);
    return gameData_;
}

void AssistProgram::setGameData(const GameData& gameData)
{
    std::lock_guard<std::mutex> locker(gameDataMtx_);
    gameData_ = gameData;
}

AssistProgramConfig AssistProgram::getConfig() const
{
    std::lock_guard<std::mutex> locker(configMtx_);
    return config_;
}

void AssistProgram::setConfig(const AssistProgramConfig& config)
{
    std::lock_guard<std::mutex> locker(configMtx_);
    config_ = config;
}

void AssistProgram::setClickKeyEnable(bool enable)
{
    enableClickKey_.store(enable);
}

bool AssistProgram::isClickKeyEnabled() const
{
    return enableClickKey_.load();
}

void AssistProgram::run()
{
    std::lock_guard<std::mutex> locker(runStopMtx_);

    if (isRunning_.load())
        return;

    shouldClose_.store(false);
    isRunning_.store(true);
    activeThreads_.store(2);

    mainWorkThread_ = std::thread([this]()
    {
        mainWork();
        if (activeThreads_.fetch_sub(1) == 1)
            isRunning_.store(false);
    });
    clickKeyWorkThread_ = std::thread([this]()
    {
        clickKeyWork();
        if (activeThreads_.fetch_sub(1) == 1)
            isRunning_.store(false);
    });
}

void AssistProgram::stop()
{
    std::lock_guard<std::mutex> locker(runStopMtx_);

    shouldClose_.store(true);

    if (mainWorkThread_.joinable())
        mainWorkThread_.join();
    if (clickKeyWorkThread_.joinable())
        clickKeyWorkThread_.join();

    activeThreads_.store(0);
    isRunning_.store(false);
}

bool AssistProgram::isRunning() const
{
    return isRunning_.load();
}

void AssistProgram::mainWork()
{
    using namespace std::chrono;

    // 获取帧
    auto getNewValidFrame = [this](NDIlib_recv_instance_t recv, uint32_t timeout) -> cv::Mat
    {
        auto startTime = high_resolution_clock::now();
        milliseconds timeoutMs(timeout);

        NDIlib_video_frame_v2_t videoFrame;
        while (!shouldClose_.load())
        {
            // 如果在指定时间内未获得帧，将返回空帧。
            if (high_resolution_clock::now() - startTime >= timeoutMs)
                break;

            auto frameType = NDIlib_recv_capture_v3(recv, &videoFrame, nullptr, nullptr, 100);
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

    // Debug使用
    size_t frameNum = 1;
    bool masterDebugWindowShowed = false;
    bool footmanDebugWindowShowed = false;

    auto lastMasterTreatTime = high_resolution_clock::now();
    auto lastFootmanTreatTime = high_resolution_clock::now();
    while (!shouldClose_.load())
    {
        GameData gameData = getGameData();
        AssistProgramConfig config = getConfig();
        milliseconds treatTimeInterval(config.treatTimeInterval);

        if (config.outputLog)
            printf("Try get frame...\n");

        cv::Mat masterFrame = getNewValidFrame(masterRecv_, config.frameGetterTimeout);
        // 如果获得空帧，退出工作线程。（下同）
        if (masterFrame.empty())
        {
            if (config.outputLog)
                printf("Failed to get master frame.\n");

            shouldClose_.store(true);
            continue;
        }

        cv::Mat footmanFrame = getNewValidFrame(footmanRecv_, config.frameGetterTimeout);
        if (footmanFrame.empty())
        {
            if (config.outputLog)
                printf("Failed to footman get frame.\n");

            shouldClose_.store(true);
            continue;
        }

        if (config.outputLog)
            printf("Got frame: %zu\n", frameNum++);

        // Master
        {
            cv::Mat hpArea = getMatView(masterFrame, gameData.hpRect);
            assert(!hpArea.empty());

            auto pos = convertProportionPosToCvPoint(gameData.hpMpBarInfo.samplingPos, hpArea.cols, hpArea.rows);
            auto color = convertCvVecToRgbColor(hpArea.at<cv::Vec3b>(pos));

            // 若采样点颜色与血条底色相同则进行治疗。（下同）
            auto similarity = computeColorSimilarity(color, gameData.hpMpBarInfo.baseColor);
            if (config.outputLog)
            {
                printf("Master HP bar color: RGB(%d, %d, %d), similarity: %lf\n",
                    static_cast<int>(color.r), static_cast<int>(color.g), static_cast<int>(color.b), similarity);
            }

            if (similarity >= config.colorConfidence &&
                (high_resolution_clock::now() - lastMasterTreatTime >= treatTimeInterval))
            {
                if (config.outputLog)
                    printf("Click F6\n");

                hid::clickKey(footmanHid_, config.masterTreatKey);
                lastMasterTreatTime = high_resolution_clock::now();
            }

            if (config.showDebugWindow)
            {
                showDebugWindow(masterFrame, "Master", gameData, config);
                masterDebugWindowShowed = true;
            }
            else
            {
                if (masterDebugWindowShowed)
                {
                    cv::destroyWindow("Master");
                    masterDebugWindowShowed = false;
                }
            }
        }

        // Footman
        {
            cv::Mat hpArea = getMatView(footmanFrame, gameData.hpRect);
            assert(!hpArea.empty());

            // 原采样点（治疗阈值）
            auto pos = convertProportionPosToCvPoint(gameData.hpMpBarInfo.samplingPos, hpArea.cols, hpArea.rows);
            // 新采样点（回城阈值）
            auto proportionPos = gameData.hpMpBarInfo.samplingPos;
            proportionPos.x = config.backHomeHpThreshold;
            auto newPos = convertProportionPosToCvPoint(proportionPos, hpArea.cols, hpArea.rows);

            auto color = convertCvVecToRgbColor(hpArea.at<cv::Vec3b>(pos));
            auto newColor = convertCvVecToRgbColor(hpArea.at<cv::Vec3b>(newPos));

            // 若新采样点颜色与血条底色相同则进行回城，并退出工作线程。
            auto newSimilarity = computeColorSimilarity(newColor, gameData.hpMpBarInfo.baseColor);
            if (config.outputLog)
            {
                printf("Footman HP bar low pos color: RGB(%d, %d, %d), similarity: %lf\n",
                    static_cast<int>(newColor.r), static_cast<int>(newColor.g), static_cast<int>(newColor.b),
                    newSimilarity);
            }

            if (newSimilarity >= config.colorConfidence)
            {
                if (config.outputLog)
                    printf("Footman HP is low, back to home and exit thread.\n");

                hid::clickKey(footmanHid_, config.backHomeKey);
                shouldClose_.store(true);
                continue;
            }

            auto similarity = computeColorSimilarity(color, gameData.hpMpBarInfo.baseColor);
            if (config.outputLog)
            {
                printf("Footman HP bar color: RGB(%d, %d, %d), similarity: %lf\n",
                    static_cast<int>(color.r), static_cast<int>(color.g), static_cast<int>(color.b), similarity);
            }

            if (similarity >= config.colorConfidence &&
                (high_resolution_clock::now() - lastFootmanTreatTime >= treatTimeInterval))
            {
                if (config.outputLog)
                    printf("Click F5\n");

                hid::clickKey(footmanHid_, config.footmanTreatKey);
                lastFootmanTreatTime = high_resolution_clock::now();
            }

            if (config.showDebugWindow)
            {
                showDebugWindow(footmanFrame, "Footman", gameData, config);
                footmanDebugWindowShowed = true;
            }
            else
            {
                if (footmanDebugWindowShowed)
                {
                    cv::destroyWindow("Footman");
                    footmanDebugWindowShowed = false;
                }
            }
        }
    }

    if (masterDebugWindowShowed)
        cv::destroyWindow("Master");
    if (footmanDebugWindowShowed)
        cv::destroyWindow("Footman");
}

void AssistProgram::clickKeyWork()
{
    using namespace std::chrono;

    auto lastClickTime = high_resolution_clock::now();
    while (!shouldClose_.load())
    {
        if (!enableClickKey_.load())
        {
            std::this_thread::sleep_for(milliseconds(50));
            lastClickTime = high_resolution_clock::now();
            continue;
        }

        AssistProgramConfig config = getConfig();
        assert(config.cps != 0);
        milliseconds clickTimeInterval(1000 / config.cps);

        auto interval = high_resolution_clock::now() - lastClickTime;
        if (interval < clickTimeInterval)
            std::this_thread::sleep_for(clickTimeInterval - interval);

        if (config.outputLog)
            printf("Click mouse left button.\n");

        hid::clickMouseButton(footmanHid_, 1);
        lastClickTime = high_resolution_clock::now();
    }
}
