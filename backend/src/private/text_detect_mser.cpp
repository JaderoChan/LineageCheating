#include "text_detect_mser.hpp"

#include <opencv2/features2d.hpp>

std::vector<cv::Rect> getTextRegions(const cv::Mat& grayImg)
{
    static cv::Ptr<cv::MSER> mser = cv::MSER::create(
        21,      // _delta
        60,      // _min_area
        14400,   // _max_area
        0.25,    // _max_variation
        0.2,     // _min_diversity
        200,     // _max_evolution
        1.01,    // _area_threshold
        0.003,   // _min_margin
        5        // _edge_blur_size
    );

    std::vector<std::vector<cv::Point>> regions;
    std::vector<cv::Rect> mserBboxes;
    mser->detectRegions(grayImg, regions, mserBboxes);

    std::vector<cv::Rect> textRegions;

    for (const auto &rect : mserBboxes)
    {
        if (rect.area() < 64)
            continue;
        float aspect = static_cast<float>(rect.width) / rect.height;
        if (aspect < 0.2f || aspect > 10.0f)
            continue;

        textRegions.push_back(rect);
    }

    return textRegions;
}
