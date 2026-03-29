#pragma once

#include <nlohmann/json.hpp>

#include "proportion_pos.hpp"
#include "proportion_rect.hpp"
#include "rgb_color.hpp"

ProportionPos loadProportionPosFromJson(const nlohmann::json& json);

ProportionRect loadProportionRectFromJson(const nlohmann::json& json);

RgbColor loadRgbColorFromJson(const nlohmann::json& json);

nlohmann::json writeProportionPosToJson(const ProportionPos& pos);

nlohmann::json writeProportionRectToJson(const ProportionRect& rect);

nlohmann::json writeRgbColorToJson(const RgbColor& color);
