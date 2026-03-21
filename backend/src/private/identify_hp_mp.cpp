#include "identify_hp_mp.hpp"

#include "ocr.hpp"

Progress identifyHpMp(const cv::Mat& img)
{
    auto text = getImageText(preprocessImageForOCR(img));
    if (text.empty())
        return Progress();
    return Progress::fromString(text);
}
