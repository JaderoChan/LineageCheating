#pragma once

#include <opencv2/opencv.hpp>

#include "tess_api.hpp"

cv::Mat preprocessImageForOCR(const cv::Mat& img);

std::string getImageText(const cv::Mat& img);
