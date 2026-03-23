#include "cheating_config.hpp"

#include <stdexcept>

#include <nlohmann/json.hpp>

/// @throw
static FrameProportionRect loadRectFromJsonObject(const nlohmann::json& j)
{
    FrameProportionRect rect;

    rect.ltx = j.at("ltx");
    rect.lty = j.at("lty");
    rect.rbx = j.at("rbx");
    rect.rby = j.at("rby");

    return rect;
}

CheatingConfig loadCheatingConfigFromJson(const std::string& json)
{
    using Json = nlohmann::json;

    CheatingConfig config;

    Json j = Json::parse(json, nullptr, false, true);
    if (j.is_discarded())
        throw std::runtime_error("Failed to parse json");

    try
    {
        config.mainRect = loadRectFromJsonObject(j.at("mainRect"));
        config.playerRect = loadRectFromJsonObject(j.at("playerRect"));
        config.hpRect = loadRectFromJsonObject(j.at("hpRect"));
        config.mpRect = loadRectFromJsonObject(j.at("mpRect"));
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error("Illegal json data");
    }

    return config;
}

DebugModeConfig loadDebugModeConfigFromJson(const std::string& json)
{
    using Json = nlohmann::json;

    DebugModeConfig config;

    Json j = Json::parse(json, nullptr, false, true);
    if (j.is_discarded())
        throw std::runtime_error("Failed to parse json");

    try
    {
        config.showWindow = j.at("showWindow");
        config.showDiffRect = j.at("showDiffRect");
        config.showTextRect = j.at("showTextRect");
        config.showDebugInfo = j.at("showDebugInfo");
        config.windowName = j.at("windowName");
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error("Illegal json data");
    }

    return config;
}
