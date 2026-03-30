#include "image_interactive_utils.hpp"

#include <algorithm>
#include <iostream>

#include <format_string.hpp>
#include <opencv_utils.hpp>

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
        cv::putText(img, text, cv::Point(5, 40), cv::FONT_HERSHEY_SIMPLEX, 1.5, hintColor, 2);

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
            case 0x0A:
                printf("[%d, %d] (%lf, %lf)\n", x, y,
                    static_cast<double>(x) / img.cols, static_cast<double>(y) / img.rows);
                break;
            default:
                break;
        }
    }

    cv::destroyWindow("Select Image Point");

    return cv::Point(x, y);
}

void cropImageByRect()
{
    std::string imgPath;
    printf("Please input source image path:\n");
    std::cin >> imgPath;

    double ltX = 0.0, ltY = 0.0;
    double rbX = 0.0, rbY = 0.0;

    printf("Please input left top x and y:\n");
    std::cin >> ltX >> ltY;

    printf("Please input right bottom x and y:\n");
    std::cin >> rbX >> rbY;

    std::string outPath;
    printf("Please input output image path:\n");
    std::cin >> outPath;

    cv::Mat image = cv::imread(imgPath);
    cv::Mat outImg = getMatView(image, ProportionRect(ltX, ltY, rbX, rbY));
    cv::imwrite(outPath, outImg);

    printf("Write image successfully!\n");
}
