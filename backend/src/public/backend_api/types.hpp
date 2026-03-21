#pragma once

struct FrameProportionRect
{
    double ltx = 0.0;
    double lty = 0.0;
    double rbx = 0.0;
    double rby = 0.0;

    bool isValid() const
    {
        return
            (ltx >= 0.0 && ltx <= 1.0) &&
            (lty >= 0.0 && lty <= 1.0) &&
            (rbx >= 0.0 && rbx <= 1.0) &&
            (rby >= 0.0 && rby <= 1.0);
    }
};
