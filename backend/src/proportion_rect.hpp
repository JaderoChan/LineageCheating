#pragma once

#include "proportion_pos.hpp"

struct ProportionRect
{
    ProportionRect() = default;
    ProportionRect(const ProportionPos& leftTop, const ProportionPos& rightBottom)
        : lt(leftTop), rb(rightBottom) {}
    ProportionRect(double ltX, double ltY, double rbX, double rbY)
        : lt(ltX, ltY), rb(rbX, rbY) {}

    double proportionWidth() const { return rb.x - lt.x; }
    double proportionHeight() const { return rb.y - lt.y; }
    bool isValid() const
    { return lt.isValid() && rb.isValid() && proportionWidth() > 0.0 && proportionHeight() > 0.0; }

    ProportionPos lt;
    ProportionPos rb;
};
