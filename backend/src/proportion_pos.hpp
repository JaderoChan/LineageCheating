#pragma once

#include <cassert>

struct ProportionPos
{
    ProportionPos() = default;
    ProportionPos(double x, double y) : x(x), y(y) {}

    /// @brief 如果成员 `x`，`y` 均位于有效范围 `[0.0, 1.0]` 中返回 `true`，否则返回 `false`。
    bool isValid() const { return x >= 0.0 && x <= 1.0 && y >= 0.0 && y <= 1.0; }

    double x = -1.0;
    double y = -1.0;
};
