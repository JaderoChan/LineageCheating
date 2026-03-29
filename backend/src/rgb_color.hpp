#pragma once

#include <cstdint>

struct RgbColor
{
    constexpr RgbColor() = default;
    constexpr RgbColor(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b) {}

    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
};

template <typename T>
constexpr T square(const T& x) { return x * x; }

/**
 * @brief 计算两种颜色的相似度。
 * @param rWeight RGB 色彩空间中，红色分量的距离权重。
 * @param gWeight RGB 色彩空间中，绿色分量的距离权重。
 * @param bWeight RGB 色彩空间中，蓝色分量的距离权重。
 * @return 两种颜色的相似度，其值域为 `[0.0, 1.0]`，两种颜色越接近，相似度越接近 `1.0`。
 */
constexpr double computeColorSimilarity(const RgbColor& lhs, const RgbColor& rhs,
    double rWeight = 1.0, double gWeight = 1.0, double bWeight = 1.0)
{
    constexpr int UINT8_MAX_SQUARE = square<int>(UINT8_MAX);

    return 1.0 - (
        square(lhs.r - rhs.r) * rWeight +
        square(lhs.g - rhs.g) * gWeight +
        square(lhs.b - rhs.b) * bWeight) /
        (rWeight + gWeight + bWeight) / UINT8_MAX_SQUARE;
}
