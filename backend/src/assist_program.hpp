#pragma once

#include <atomic>
#include <functional>
#include <mutex>
#include <thread>

#include <Processing.NDI.Lib.h>
#include <opencv2/opencv.hpp>

#include "assist_program_config.hpp"
#include "game_data.hpp"
#include "hid_api.hpp"

class AssistProgram
{
public:
    AssistProgram(
        NDIlib_recv_instance_t masterRecv, NDIlib_recv_instance_t footmanRecv, hid::HID footmanHid,
        const GameData& gameData, const AssistProgramConfig& config,
        const std::function<void (const cv::Mat& masterDebugFrame, const cv::Mat& footmanDebugFrame)>& = nullptr);
    ~AssistProgram();

    GameData getGameData() const;
    void setGameData(const GameData& gameData);

    AssistProgramConfig getConfig() const;
    void setConfig(const AssistProgramConfig& config);

    void setClickKeyEnable(bool enable);
    bool isClickKeyEnabled() const;

    void run();
    void stop();

    bool isRunning() const;

private:
    void mainWork();
    void clickKeyWork();

    std::atomic<int> activeThreads_;
    std::thread mainWorkThread_;
    std::thread clickKeyWorkThread_;

    mutable std::mutex runStopMtx_;
    std::atomic<bool> shouldClose_;
    std::atomic<bool> isRunning_;

    mutable std::mutex gameDataMtx_;
    GameData gameData_;

    mutable std::mutex configMtx_;
    AssistProgramConfig config_;

    std::atomic<bool> enableClickKey_{true};

    NDIlib_recv_instance_t masterRecv_;
    NDIlib_recv_instance_t footmanRecv_;
    hid::HID footmanHid_;

    std::function<void (const cv::Mat& masterDebugFrame, const cv::Mat& footmanDebugFrame)>
    debugFrameCallback_ = nullptr;
};
