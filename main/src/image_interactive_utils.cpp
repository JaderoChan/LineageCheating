#include "image_interactive_utils.hpp"

#include <algorithm>

#include <backend_api/format_string.hpp>

/**
 * @brief Interactively select points in the image.
 * @note Use `WASD (normal speed)` / `IJKL (fast)` to move the position,
 * press `ESC` to exit and return to point coordinates.
 */
cv::Point selectImagePoint(const cv::Mat& image, int originX, int originY, const cv::Scalar& hintColor)
{
    int x = originX, y = originY;
    bool shouldClose = false;

    while (!shouldClose)
    {
        cv::Mat img = image.clone();

        cv::line(img, cv::Point(0, y), cv::Point(img.cols, y), hintColor);  // Horizontal
        cv::line(img, cv::Point(x, 0), cv::Point(x, img.rows), hintColor);  // Vertical
        std::string text = formatString(
            "[{}, {}] ({}, {})",
            std::to_string(x),
            std::to_string(y),
            std::to_string(static_cast<double>(x) / img.cols),
            std::to_string(static_cast<double>(y) / img.rows)
        );
        cv::putText(img, text, cv::Point(5, 20), cv::FONT_HERSHEY_SIMPLEX, 1.0, hintColor);

        std::string winName ="Select Image Point";
        cv::namedWindow(winName, cv::WINDOW_NORMAL);
        cv::setWindowProperty(winName, cv::WND_PROP_ASPECT_RATIO, cv::WINDOW_KEEPRATIO);
        cv::setWindowProperty(winName, cv::WND_PROP_FULLSCREEN, cv::WINDOW_FULLSCREEN);
        cv::imshow(winName, img);

        int key = cv::waitKey(0);
        switch (key)
        {
            case 'a':
            case 'j':
                x = std::clamp(x - (key == 'a' ? 1 : 10), 0, img.cols);
                break;
            case 'w':
            case 'i':
                y = std::clamp(y - (key == 'w' ? 1 : 10), 0, img.rows);
                break;
            case 'd':
            case 'l':
                x = std::clamp(x + (key == 'd' ? 1 : 10), 0, img.cols);
                break;
            case 's':
            case 'k':
                y = std::clamp(y + (key == 's' ? 1 : 10), 0, img.rows);
                break;
            case 0x1B:
                shouldClose = true;
                break;
            default:
                break;
        }
    }

    cv::destroyWindow("Select Image Point");

    return cv::Point(x, y);
}

void selectImagePointToolWrapper()
{}
