#pragma once

#include <opencv2/opencv.hpp>

#include <backend_api/progress.hpp>

Progress identifyHpMp(const cv::Mat& img);
