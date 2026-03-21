#include "ocr.hpp"

cv::Mat preprocessImageForOCR(const cv::Mat& img)
{
    cv::Mat processed;

    if (img.channels() != 1)
        cv::cvtColor(img, processed, cv::COLOR_BGR2GRAY);
    else
        processed = img.clone();

    double scale = 2.0;
    cv::resize(processed, processed, cv::Size(), scale, scale, cv::INTER_CUBIC);

    cv::fastNlMeansDenoising(processed, processed, 10, 7, 21);

    cv::adaptiveThreshold(
        processed, processed, 255,
        cv::ADAPTIVE_THRESH_GAUSSIAN_C,
        cv::THRESH_BINARY, 11, 2
    );

    static cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(2, 2));
    cv::morphologyEx(processed, processed, cv::MORPH_CLOSE, kernel);

    return processed;
}

std::string getImageText(const cv::Mat& img)
{
    auto& tess = TessAPI::getInstance()();
    std::string res;

    tess.SetImage(img.data, img.cols, img.rows, 3, img.step);
    char* text = tess.GetUTF8Text();
    if (text)
        res = text;
    delete[] text;

    return res;
}
