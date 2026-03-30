#pragma once

#include <opencv2/opencv.hpp>

#include "data_struct_converter.hpp"

inline cv::Mat getMatView(cv::Mat& mat, const ProportionRect& rect)
{
    assert(rect.isValid());
    return mat(convertProportionRectToCvRect(rect, mat.cols, mat.rows));
}

cv::Mat limitImageSize(const cv::Mat& src, int maxWidth, int maxHeight);
