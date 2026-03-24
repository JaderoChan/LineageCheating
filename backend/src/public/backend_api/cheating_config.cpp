#include "cheating_config.hpp"

#include <stdexcept>

#include <nlohmann/json.hpp>

CheatingConfig loadCheatingConfigFromJson(const std::string& json)
{
    using Json = nlohmann::json;

    CheatingConfig config;

    Json j = Json::parse(json, nullptr, false, true);
    if (j.is_discarded())
        throw std::runtime_error("Failed to parse json");

    try
    {
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error("Illegal json data");
    }

    return config;
}
