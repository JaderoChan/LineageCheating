#pragma once

#include <opencv2/opencv.hpp>

cv::Point selectImagePoint(const cv::Mat& image, int originX = 0, int originY = 0,
    const cv::Scalar& hintColor = cv::Scalar(0, 0, 255));
