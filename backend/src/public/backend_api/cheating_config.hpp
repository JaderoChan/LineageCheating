#pragma once

#include <string>

#include "types.hpp"

struct CheatingConfig
{
    FrameProportionRect mainRect = {
        0.139204,
        0.0,
        0.790563,
        0.735465
    };
    FrameProportionRect playerRect = {
        0.478906,
        0.303779,
        0.521094,
        0.415698
    };
    FrameProportionRect hpRect = {
        0.397344,
        0.793605,
        0.444531,
        0.812500
    };
    FrameProportionRect mpRect = {
        0.542578,
        0.793605,
        0.575712,
        0.812500
    };
    FrameProportionRect arrowRect = {
        0.837560,
        0.915972,
        0.866036,
        0.936711
    };

    int textRegionExpansionX = 50;
    int textRegionExpansionY = 50;

    // unit in millisecond
    int sleepAfterMove = 20;
    int arrowUnchangedTimeThreshold = 800;
    int fps = 15;

    double hpThresholdPercent = 0.3;
};

struct DebugModeConfig
{
    bool showWindow         = false;
    bool showDiffRect       = true;
    bool showTextRect       = true;
    bool showDebugInfo      = true;
    int windowMaxX          = 1440;
    int windowMaxY          = 1440;
    std::string windowName;
};

/// @throw std::runtime_error
CheatingConfig loadCheatingConfigFromJson(const std::string& json);

/// @throw std::runtime_error
DebugModeConfig loadDebugModeConfigFromJson(const std::string& json);
