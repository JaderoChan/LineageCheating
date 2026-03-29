#include "opencv_utils.hpp"

#include <stdexcept>

cv::Mat limitImageSize(const cv::Mat& src, int maxWidth, int maxHeight)
{
    if (maxWidth <= 0 && maxHeight <= 0)
        throw std::runtime_error("Invalid argument");

    if ((src.cols <= maxWidth && src.rows <= maxHeight) || (src.cols == 0 || src.rows == 0))
        return src.clone();

    double xRatio = static_cast<double>(maxWidth) / src.cols;
    double yRatio = static_cast<double>(maxHeight) / src.rows;
    double ratio = std::min(xRatio, yRatio);

    int newCols = src.cols * ratio;
    int newRows = src.rows * ratio;
    if (newCols <= 0 || newRows <= 0)
        throw std::runtime_error("Failed to limit image size");

    cv::Mat limitedImg;
    cv::resize(src, limitedImg, cv::Size(newCols, newRows));

    return limitedImg;
}
