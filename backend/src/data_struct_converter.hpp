#pragma once

#include <cassert>

#include <opencv2/opencv.hpp>

#include "proportion_pos.hpp"
#include "proportion_rect.hpp"
#include "rgb_color.hpp"

inline cv::Point convertProportionPosToCvPoint(const ProportionPos& pos, int width, int height)
{
    assert(width > 0 && height > 0);
    assert(pos.isValid());

    int x = pos.x * width;
    int y = pos.y * height;

    return cv::Point(x, y);
}

inline ProportionPos convertCvPointToProportionPos(const cv::Point& pos, int width, int height)
{
    assert(width > 0 && height > 0);

    double x = static_cast<double>(pos.x) / width;
    double y = static_cast<double>(pos.y) / height;

    return ProportionPos(x, y);
}

inline cv::Rect convertProportionRectToCvRect(const ProportionRect& rect, int width, int height)
{
    assert(width > 0 && height > 0);
    assert(rect.isValid());

    cv::Point lt(rect.lt.x * width, rect.lt.y * height);
    cv::Point rb(rect.rb.x * width, rect.rb.y * height);

    return cv::Rect(lt, rb);
}

inline ProportionRect convertCvRectToProportionRect(const cv::Rect& rect, int width, int height)
{
    assert(width > 0 && height > 0);

    auto tl = rect.tl();
    assert(tl.x >= 0 && tl.x <= width && tl.y >= 0 && tl.y <= height);
    auto br = rect.br();
    assert(br.x >= 0 && br.x <= width && br.y >= 0 && br.y <= height);

    double ltX = static_cast<double>(tl.x) / width;
    double ltY = static_cast<double>(tl.y) / height;
    double rbX = static_cast<double>(br.x) / width;
    double rbY = static_cast<double>(br.y) / height;

    return ProportionRect(ltX, ltY, rbX, rbY);
}

inline cv::Vec3b convertRgbColorToCvVec(const RgbColor& color)
{
    return cv::Vec3b(color.b, color.g, color.r);
}

inline RgbColor convertCvVecToRgbColor(const cv::Vec3b& color)
{
    return RgbColor(color[2], color[1], color[0]);
}
