#pragma once

#include <vector>

#include <opencv2/opencv.hpp>

#include <backend_api/types.hpp>

cv::Mat limitImageSize(const cv::Mat& src, int maxX, int maxY);

cv::Mat cropImageProportioned(const cv::Mat& src, const FrameProportionRect& rect);

std::vector<cv::Rect> getImageDiffs(const cv::Mat& a, const cv::Mat& b, double minimumArea = 64.0);
