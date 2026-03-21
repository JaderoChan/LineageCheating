#include "game_frame_utils.hpp"

GameFrameUtils::GameFrameUtils(const CheatingConfig& config)
    : config_(config)
{}

void GameFrameUtils::setConfig(const CheatingConfig& config)
{
    config_.store(config);
}

void GameFrameUtils::setGameFrame(cv::Mat* gameFrame)
{
    gameFrame_ = gameFrame;
}

void GameFrameUtils::unsetGameFrame()
{
    gameFrame_ = nullptr;
}

cv::Mat GameFrameUtils::getMainGameAreaFrame() const
{
    if (!gameFrame_)
        return cv::Mat();
    return cropImageProportioned(*gameFrame_, config_.load().mainRect);
}

cv::Mat GameFrameUtils::getPlayerAreaFrame() const
{
    if (!gameFrame_)
        return cv::Mat();
    return cropImageProportioned(*gameFrame_, config_.load().playerRect);
}

cv::Mat GameFrameUtils::getHpAreaFrame() const
{
    if (!gameFrame_)
        return cv::Mat();
    return cropImageProportioned(*gameFrame_, config_.load().hpRect);
}

cv::Mat GameFrameUtils::getMpAreaFrame() const
{
    if (!gameFrame_)
        return cv::Mat();
    return cropImageProportioned(*gameFrame_, config_.load().mpRect);
}

cv::Rect GameFrameUtils::mapMainGameAreaRectToSource(const cv::Rect& rect) const
{
    const auto& mainRect = config_.load().mainRect;
    cv::Point lt(
        static_cast<int>(std::round(mainRect.ltx * gameFrame_->cols)),
        static_cast<int>(std::round(mainRect.lty * gameFrame_->rows)));
    return cv::Rect(lt + rect.tl(), rect.size());
}

void GameFrameUtils::setPlayerAreaToBlack()
{
    if (!gameFrame_)
        return;
    getPlayerAreaFrame().setTo(cv::Scalar(0.0, 0.0, 0.0));
}
