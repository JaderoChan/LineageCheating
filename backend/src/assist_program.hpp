#pragma once

#include <atomic>
#include <mutex>
#include <thread>

#include <Processing.NDI.Lib.h>

#include "assist_program_config.hpp"
#include "game_data.hpp"
#include "hid_api.hpp"

class AssistProgram
{
public:
    AssistProgram(
        NDIlib_recv_instance_t masterRecv, NDIlib_recv_instance_t footmanRecv, hid::HID footmanHid,
        const GameData& gameData, const AssistProgramConfig& config);
    ~AssistProgram();

    void updateGameData(const GameData& gameData);
    void updateConfig(const AssistProgramConfig& config);

    GameData getGameData() const;
    AssistProgramConfig getConfig() const;

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
};
