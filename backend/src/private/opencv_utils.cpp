#include "opencv_utils.hpp"

#include <cassert>
#include <cmath>

cv::Mat limitImageSize(const cv::Mat& src, int maxX, int maxY)
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

cv::Mat cropImageProportioned(const cv::Mat& src, const FrameProportionRect& rect)
{
    if (src.empty())
        return src;

    assert(rect.isValid());

    cv::Point lt(
        static_cast<int>(std::round(rect.ltx * src.cols)),
        static_cast<int>(std::round(rect.lty * src.rows)));
    cv::Point rb(
        static_cast<int>(std::round(rect.rbx * src.cols)),
        static_cast<int>(std::round(rect.rby * src.rows)));
    cv::Rect roi(lt, rb);

    return src(roi);
}

std::vector<cv::Rect> getImageDiffs(const cv::Mat& a, const cv::Mat& b, double minimumArea)
{
    if (a.empty() || b.empty())
        return std::vector<cv::Rect>();

    cv::Mat aGray;
    cv::Mat bGray;

    cv::cvtColor(a, aGray, cv::COLOR_BGR2GRAY);
    cv::GaussianBlur(aGray, aGray, cv::Size(7, 7), 0);

    cv::cvtColor(b, bGray, cv::COLOR_BGR2GRAY);
    cv::GaussianBlur(bGray, bGray, cv::Size(7, 7), 0);

    cv::Mat diff;
    cv::absdiff(aGray, bGray, diff);

    cv::Mat thresh;
    cv::threshold(diff, thresh, 25, 255, cv::THRESH_BINARY);

    cv::dilate(thresh, thresh, cv::Mat(), cv::Point(-1, -1), 2);

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(thresh, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    std::vector<cv::Rect> diffRects;
    for (const auto& pts : contours)
    {
        if (cv::contourArea(pts) < minimumArea)
            continue;
        diffRects.emplace_back(cv::boundingRect(pts));
    }

    return diffRects;
}
