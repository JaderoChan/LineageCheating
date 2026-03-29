#include "data_struct_rw.hpp"

#include <stdexcept>

#define THROW_JSON_ILLEGAL throw std::runtime_error("Given json is illegal in function " __FUNCTION__)

ProportionPos loadProportionPosFromJson(const nlohmann::json& json)
{
    if (!json.is_array() || json.size() != 2)
        THROW_JSON_ILLEGAL;

    auto& ele1 = json.at(0);
    auto& ele2 = json.at(1);
    if (!ele1.is_number_float() || !ele2.is_number_float())
        THROW_JSON_ILLEGAL;

    ProportionPos result;
    result.x = ele1;
    result.y = ele2;

    return result;
}

ProportionRect loadProportionRectFromJson(const nlohmann::json& json)
{
    if (!json.is_array() || json.size() != 4)
        THROW_JSON_ILLEGAL;

    auto& ele1 = json.at(0);
    auto& ele2 = json.at(1);
    auto& ele3 = json.at(2);
    auto& ele4 = json.at(3);
    if (!ele1.is_number_float() || !ele2.is_number_float() || !ele3.is_number_float() || !ele4.is_number_float())
        THROW_JSON_ILLEGAL;

    ProportionRect result;
    result.lt.x = ele1;
    result.lt.y = ele2;
    result.rb.x = ele3;
    result.rb.y = ele4;

    return result;
}

RgbColor loadRgbColorFromJson(const nlohmann::json& json)
{
    if (!json.is_array() || json.size() != 3)
        THROW_JSON_ILLEGAL;

    auto& ele1 = json.at(0);
    auto& ele2 = json.at(1);
    auto& ele3 = json.at(2);
    if (!ele1.is_number_unsigned() || !ele2.is_number_unsigned() || !ele3.is_number_unsigned())
        THROW_JSON_ILLEGAL;

    RgbColor result;
    result.r = ele1;
    result.g = ele2;
    result.b = ele3;

    return result;
}

nlohmann::json writeProportionPosToJson(const ProportionPos& pos)
{
    return nlohmann::json::array({pos.x, pos.y});
}

nlohmann::json writeProportionRectToJson(const ProportionRect& rect)
{
    return nlohmann::json::array({rect.lt.x, rect.lt.y, rect.rb.x, rect.rb.y});
}

nlohmann::json writeRgbColorToJson(const RgbColor& color)
{
    return nlohmann::json::array({color.r, color.g, color.b});
}
