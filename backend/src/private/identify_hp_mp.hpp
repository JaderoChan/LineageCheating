#pragma once

#include <opencv2/opencv.hpp>

#include <backend_api/progress.hpp>

struct HpMp
{
    Progress hp;
    Progress mp;
};

HpMp identifyHpMp(const cv::Mat& img);
