#include "identifyHpMp.hpp"

#include "ocr.hpp"

Progress identifyHpMp(const cv::Mat& img)
{
    auto text = getImageText(img);
    if (text.empty())
        return Progress();
    return Progress::fromString(text);
}
