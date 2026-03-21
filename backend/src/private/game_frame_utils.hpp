#pragma once

#include <atomic>

#include <backend_api/cheating_config.hpp>

#include "opencv_utils.hpp"

class GameFrameUtils
{
public:
    GameFrameUtils(const CheatingConfig& config);

    void setConfig(const CheatingConfig& config);
    void setGameFrame(cv::Mat* gameFrame);
    void unsetGameFrame();

    cv::Mat getMainGameAreaFrame() const;
    cv::Mat getPlayerAreaFrame() const;
    cv::Mat getHpAreaFrame() const;
    cv::Mat getMpAreaFrame() const;

    cv::Rect mapMainGameAreaRectToSource(const cv::Rect& rect) const;

    void setPlayerAreaToBlack();

private:
    std::atomic<CheatingConfig> config_{};
    cv::Mat* gameFrame_ = nullptr;
};
