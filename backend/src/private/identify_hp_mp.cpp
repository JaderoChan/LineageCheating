#include "identify_hp_mp.hpp"

#include "ocr.hpp"

HpMp identifyHpMp(const cv::Mat& img)
{
    if (!img.empty())
    {
        HpMp hpmp;

        auto& ocrLite = getOcrLiteInstance();
        auto result = ocrLite.detect(img, 50, 1024, 0.35, 0.3, 1.6, false, false).strRes;
        std::string process;
        for (char ch : result)
        {
            if (std::isblank(ch))
                continue;
            process += ch;
        }

        auto hpPos = process.find("HP:");
        if (hpPos != std::string::npos && hpPos + 3 < process.size())
        {
            hpPos += 3;
            size_t endPos = hpPos;
            while (endPos < process.size() && std::isdigit(process[endPos]) || process[endPos] == '/') endPos++;
            hpmp.hp = Progress::fromString(process.substr(hpPos, endPos - hpPos));
        }

        auto mpPos = process.find("MP:");
        if (mpPos != std::string::npos && mpPos + 3 < process.size())
        {
            mpPos += 3;
            size_t endPos = mpPos;
            while (endPos < process.size() && std::isdigit(process[endPos]) || process[endPos] == '/') endPos++;
            hpmp.mp = Progress::fromString(process.substr(mpPos, endPos - mpPos));
        }

        return hpmp;
    }
    return HpMp();
}
