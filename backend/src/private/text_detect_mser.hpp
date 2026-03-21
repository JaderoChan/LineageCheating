#include <vector>

#include <opencv2/opencv.hpp>

std::vector<cv::Rect> getTextRegions(const cv::Mat& grayImg);
