#pragma once

#include <opencv2/opencv.hpp>

#include <OcrLite.h>

OcrLite& getOcrLiteInstance();

std::string getImageText(const cv::Mat& img);
