#pragma once

#include <string>

#include "types.hpp"

struct CheatingConfig
{
    FrameProportionRect mainRect = {
        0.179204,
        0.0,
        0.867257,
        0.728863
    };
    FrameProportionRect playerRect = {
        0.471238,
        0.291545,
        0.533186,
        0.408163
    };
    FrameProportionRect hpRect = {
        0.341814,
        0.787172,
        0.423673,
        0.804665
    };
    FrameProportionRect mpRect = {
        0.538292,
        0.787172,
        0.605044,
        0.804665
    };

    int textRegionExpansionX = 50;
    int textRegionExpansionY = 50;

    int sleepAfterMove = 100;

    int continuousAttackTime = 0;   // unit in millisecond
};

struct DebugModeConfig
{
    bool showWindow = false;
    bool showDiffRect = true;
    bool showTextRect = true;
    bool showHpMp = true;
    std::string windowName;
};

/// @throw std::runtime_error
CheatingConfig loadCheatingConfigFromJson(const std::string& json);

/// @throw std::runtime_error
DebugModeConfig loadDebugModeConfigFromJson(const std::string& json);
