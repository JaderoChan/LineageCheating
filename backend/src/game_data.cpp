#include "game_data.hpp"

#include <fstream>

#define READ_FIELD_FROM_JSON(struct, field, json, defaultValue) (struct).field = (json).value(#field, (defaultValue))
#define READ_PROPORTION_RECT_FIELD_FROM_JSON(struct, field, json) \
(struct).field = loadProportionRectFromJson((json).value(#field, nlohmann::json::object()))
#define READ_PROPORTION_POS_FIELD_FROM_JSON(struct, field, json) \
(struct).field = loadProportionPosFromJson((json).value(#field, nlohmann::json::object()))
#define READ_RGB_COLOR_FIELD_FROM_JSON(struct, field, json) \
(struct).field = loadRgbColorFromJson((json).value(#field, nlohmann::json::object()))

#define WRITE_FIELD_TO_JSON(struct, field, json) (json)[#field] = (struct).field
#define WRITE_PROPORTION_RECT_FIELD_TO_JSON(struct, field, json) \
(json)[#field] = writeProportionRectToJson((struct).field)
#define WRITE_PROPORTION_POS_FIELD_TO_JSON(struct, field, json) \
(json)[#field] = writeProportionPosToJson((struct).field)
#define WRITE_RGB_COLOR_FIELD_TO_JSON(struct, field, json) \
(json)[#field] = writeRgbColorToJson((struct).field)

GameData GameData::fromJson(const nlohmann::json& json)
{
    GameData result;

    READ_PROPORTION_RECT_FIELD_FROM_JSON(result, mainRect, json);
    READ_PROPORTION_RECT_FIELD_FROM_JSON(result, playerRect, json);
    READ_PROPORTION_RECT_FIELD_FROM_JSON(result, bottomRect, json);
    READ_PROPORTION_RECT_FIELD_FROM_JSON(result, buffRect, json);
    READ_PROPORTION_RECT_FIELD_FROM_JSON(result, popupMenuRect, json);
    READ_PROPORTION_RECT_FIELD_FROM_JSON(result, hpRect, json);
    READ_PROPORTION_RECT_FIELD_FROM_JSON(result, mpRect, json);
    READ_PROPORTION_RECT_FIELD_FROM_JSON(result, chatRect, json);

    READ_PROPORTION_RECT_FIELD_FROM_JSON(result, hotkeyRects.f4, json);
    READ_PROPORTION_RECT_FIELD_FROM_JSON(result, hotkeyRects.f5, json);
    READ_PROPORTION_RECT_FIELD_FROM_JSON(result, hotkeyRects.f6, json);
    READ_PROPORTION_RECT_FIELD_FROM_JSON(result, hotkeyRects.f7, json);
    READ_PROPORTION_RECT_FIELD_FROM_JSON(result, hotkeyRects.f8, json);
    READ_PROPORTION_RECT_FIELD_FROM_JSON(result, hotkeyRects.f9, json);

    READ_PROPORTION_POS_FIELD_FROM_JSON(result, hpMpBarInfo.samplingPos, json);
    READ_RGB_COLOR_FIELD_FROM_JSON(result, hpMpBarInfo.baseColor, json);
    READ_RGB_COLOR_FIELD_FROM_JSON(result, hpMpBarInfo.hpColor, json);
    READ_RGB_COLOR_FIELD_FROM_JSON(result, hpMpBarInfo.mpColor, json);

    return result;
}

GameData GameData::fromFile(const std::string& filepath)
{
    std::ifstream ifs(filepath);
    if (!ifs.is_open())
        throw std::runtime_error("Failed to open file: " + filepath);

    nlohmann::json j = nlohmann::json::parse(ifs, nullptr, false, true);
    ifs.close();

    if (j.is_discarded())
        throw std::runtime_error("Failed to parse json file: " + filepath);

    return fromJson(j);
}

nlohmann::json GameData::toJson() const
{
    nlohmann::json j;

    WRITE_PROPORTION_RECT_FIELD_TO_JSON(*this, mainRect, j);
    WRITE_PROPORTION_RECT_FIELD_TO_JSON(*this, playerRect, j);
    WRITE_PROPORTION_RECT_FIELD_TO_JSON(*this, bottomRect, j);
    WRITE_PROPORTION_RECT_FIELD_TO_JSON(*this, buffRect, j);
    WRITE_PROPORTION_RECT_FIELD_TO_JSON(*this, popupMenuRect, j);
    WRITE_PROPORTION_RECT_FIELD_TO_JSON(*this, hpRect, j);
    WRITE_PROPORTION_RECT_FIELD_TO_JSON(*this, mpRect, j);
    WRITE_PROPORTION_RECT_FIELD_TO_JSON(*this, chatRect, j);

    WRITE_PROPORTION_RECT_FIELD_TO_JSON(*this, hotkeyRects.f4, j);
    WRITE_PROPORTION_RECT_FIELD_TO_JSON(*this, hotkeyRects.f5, j);
    WRITE_PROPORTION_RECT_FIELD_TO_JSON(*this, hotkeyRects.f6, j);
    WRITE_PROPORTION_RECT_FIELD_TO_JSON(*this, hotkeyRects.f7, j);
    WRITE_PROPORTION_RECT_FIELD_TO_JSON(*this, hotkeyRects.f8, j);
    WRITE_PROPORTION_RECT_FIELD_TO_JSON(*this, hotkeyRects.f9, j);

    WRITE_PROPORTION_POS_FIELD_TO_JSON(*this, hpMpBarInfo.samplingPos, j);
    WRITE_RGB_COLOR_FIELD_TO_JSON(*this, hpMpBarInfo.baseColor, j);
    WRITE_RGB_COLOR_FIELD_TO_JSON(*this, hpMpBarInfo.hpColor, j);
    WRITE_RGB_COLOR_FIELD_TO_JSON(*this, hpMpBarInfo.mpColor, j);

    return j;
}

void GameData::toFile(const std::string& filepath) const
{
    std::string jsonStr = toJson().dump(4);

    std::ofstream ofs(filepath);
    if (!ofs.is_open())
        throw std::runtime_error("Failed to open file: " + filepath);

    ofs << jsonStr;
    ofs.close();
}
