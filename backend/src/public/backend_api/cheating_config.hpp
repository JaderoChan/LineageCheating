#pragma once

#include <string>

struct CheatingConfig
{
    // unit in millisecond
    int heartTimeInterval = 1000;

    int fps = 10;
    int cps = 10;

    double hpFlagPointX = 0.337578;
    double hpFlagPointY = 0.804167;

    struct Color
    {
        unsigned char r = 0x94;
        unsigned char g = 0x8b;
        unsigned char b = 0x8a;
    } hpThresoldColor;
};

/// @throw std::runtime_error
CheatingConfig loadCheatingConfigFromJson(const std::string& json);
