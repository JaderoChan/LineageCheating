#pragma once

#include <opencv2/opencv.hpp>

cv::Mat limitImageSize(const cv::Mat& src, int maxWidth, int maxHeight);
