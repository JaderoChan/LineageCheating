#pragma once

#include <backend_api/cheating_config.hpp>

#include "opencv_utils.hpp"

class GameFrameUtils
{
public:
    GameFrameUtils(const CheatingConfig& config);

    cv::Mat getMainGameAreaFrame(const cv::Mat& frame) const;
    cv::Mat getPlayerAreaFrame(const cv::Mat& frame) const;
    cv::Mat getHpAreaFrame(const cv::Mat& frame) const;
    cv::Mat getMpAreaFrame(const cv::Mat& frame) const;

    cv::Mat getHandledMainGameAreaFrame(cv::Mat& frame) const;

    cv::Rect mapMainGameAreaRectToSource(const cv::Mat& frame, const cv::Rect& rect) const;

    void setPlayerAreaToBlack(cv::Mat& frame) const;

private:
    CheatingConfig config_;
};
