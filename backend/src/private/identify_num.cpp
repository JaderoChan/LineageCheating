#include "identify_num.hpp"

#include "ocr.hpp"

int identifyNum(const cv::Mat& img)
{
    if (img.empty())
        return -1;

    auto text = getImageText(img);
    if (text.empty())
        return -1;

    std::string processed;
    for (char ch : text)
    {
        if (isdigit(ch))
            processed += ch;
    }

    try { return std::stoi(processed); }
    catch (...) { return -1; }
}
