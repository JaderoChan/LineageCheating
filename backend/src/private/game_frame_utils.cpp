#include "game_frame_utils.hpp"

GameFrameUtils::GameFrameUtils(const CheatingConfig& config)
    : config_(config)
{}

cv::Mat GameFrameUtils::getMainGameAreaFrame(const cv::Mat& frame) const
{
    return cropImageProportioned(frame, config_.mainRect);
}

cv::Mat GameFrameUtils::getPlayerAreaFrame(const cv::Mat& frame) const
{
    return cropImageProportioned(frame, config_.playerRect);
}

cv::Mat GameFrameUtils::getHpMpAreaFrame(const cv::Mat& frame) const
{
    return cropImageProportioned(frame, config_.hpMpRect);
}

cv::Mat GameFrameUtils::getArrowAreaFrame(const cv::Mat& frame) const
{
    return cropImageProportioned(frame, config_.arrowRect);
}

cv::Mat GameFrameUtils::getHandledMainGameAreaFrame(cv::Mat& frame) const
{
    setPlayerAreaToBlack(frame);
    return getMainGameAreaFrame(frame);
}

cv::Rect GameFrameUtils::mapMainGameAreaRectToSource(const cv::Mat& frame, const cv::Rect& rect) const
{
    const auto& mainRect = config_.mainRect;
    cv::Point lt(
        static_cast<int>(std::round(mainRect.ltx * frame.cols)),
        static_cast<int>(std::round(mainRect.lty * frame.rows)));
    return cv::Rect(lt + rect.tl(), rect.size());
}

void GameFrameUtils::setPlayerAreaToBlack(cv::Mat& frame) const
{
    getPlayerAreaFrame(frame).setTo(cv::Scalar(0.0, 0.0, 0.0));
}
