#include "opencv_utils.hpp"

#include <cassert>
#include <cmath>

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
    static cv::Mat aGray;
    static cv::Mat bGray;

    cv::cvtColor(a, aGray, cv::COLOR_BGR2GRAY);
    cv::GaussianBlur(aGray, aGray, cv::Size(7, 7), 0);

    cv::cvtColor(b, bGray, cv::COLOR_BGR2GRAY);
    cv::GaussianBlur(bGray, bGray, cv::Size(7, 7), 0);

    static cv::Mat diff;
    cv::absdiff(aGray, bGray, diff);

    static cv::Mat thresh;
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
