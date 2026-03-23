#include "identify_hp_mp.hpp"

#include "ocr.hpp"

Progress identifyHpMp(const cv::Mat& img)
{
    if (!img.empty())
    {
        auto text = getImageText(img);
        if (!text.empty())
            return Progress::fromString(text);
    }
    return Progress();
}
