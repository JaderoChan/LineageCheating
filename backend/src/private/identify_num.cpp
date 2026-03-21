#include "identify_num.hpp"

#include "ocr.hpp"

int identifyNum(const cv::Mat& img)
{
    auto text = getImageText(preprocessImageForOCR(img));
    if (text.empty())
        return -1;

    try
    {
        return std::stoi(text);
    }
    catch (...)
    {
        return -1;
    }
}
