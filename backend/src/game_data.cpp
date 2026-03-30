#include "game_data.hpp"

#include <fstream>

#define READ_FIELD_FROM_JSON(struct, field, jsonObj, defaultValue) (struct).field = (jsonObj).value(#field, (defaultValue))
#define READ_PROPORTION_RECT_FIELD_FROM_JSON(struct, field, jsonObj) \
(struct).field = loadProportionRectFromJson((jsonObj).at(#field))
#define READ_PROPORTION_POS_FIELD_FROM_JSON(struct, field, jsonObj) \
(struct).field = loadProportionPosFromJson((jsonObj).at(#field))
#define READ_RGB_COLOR_FIELD_FROM_JSON(struct, field, jsonObj) \
(struct).field = loadRgbColorFromJson((jsonObj).at(#field))

#define WRITE_FIELD_TO_JSON(struct, field, jsonObj) (jsonObj)[#field] = (struct).field
#define WRITE_PROPORTION_RECT_FIELD_TO_JSON(struct, field, jsonObj) \
(jsonObj)[#field] = writeProportionRectToJson((struct).field)
#define WRITE_PROPORTION_POS_FIELD_TO_JSON(struct, field, jsonObj) \
(jsonObj)[#field] = writeProportionPosToJson((struct).field)
#define WRITE_RGB_COLOR_FIELD_TO_JSON(struct, field, jsonObj) \
(jsonObj)[#field] = writeRgbColorToJson((struct).field)

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

    nlohmann::json hotkeyRectsObj = json.at("hotkeyRects");
    READ_PROPORTION_RECT_FIELD_FROM_JSON(result.hotkeyRects, f5, hotkeyRectsObj);
    READ_PROPORTION_RECT_FIELD_FROM_JSON(result.hotkeyRects, f6, hotkeyRectsObj);
    READ_PROPORTION_RECT_FIELD_FROM_JSON(result.hotkeyRects, f7, hotkeyRectsObj);
    READ_PROPORTION_RECT_FIELD_FROM_JSON(result.hotkeyRects, f8, hotkeyRectsObj);
    READ_PROPORTION_RECT_FIELD_FROM_JSON(result.hotkeyRects, f9, hotkeyRectsObj);
    READ_PROPORTION_RECT_FIELD_FROM_JSON(result.hotkeyRects, f10, hotkeyRectsObj);
    READ_PROPORTION_RECT_FIELD_FROM_JSON(result.hotkeyRects, f11, hotkeyRectsObj);
    READ_PROPORTION_RECT_FIELD_FROM_JSON(result.hotkeyRects, f12, hotkeyRectsObj);

    nlohmann::json hpMpBarInfoObj = json.at("hpMpBarInfo");
    READ_PROPORTION_POS_FIELD_FROM_JSON(result.hpMpBarInfo, samplingPos, hpMpBarInfoObj);
    READ_RGB_COLOR_FIELD_FROM_JSON(result.hpMpBarInfo, baseColor, hpMpBarInfoObj);
    READ_RGB_COLOR_FIELD_FROM_JSON(result.hpMpBarInfo, hpColor, hpMpBarInfoObj);
    READ_RGB_COLOR_FIELD_FROM_JSON(result.hpMpBarInfo, mpColor, hpMpBarInfoObj);

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

    nlohmann::json hotkeyRectsObj;
    WRITE_PROPORTION_RECT_FIELD_TO_JSON(this->hotkeyRects, f5, hotkeyRectsObj);
    WRITE_PROPORTION_RECT_FIELD_TO_JSON(this->hotkeyRects, f6, hotkeyRectsObj);
    WRITE_PROPORTION_RECT_FIELD_TO_JSON(this->hotkeyRects, f7, hotkeyRectsObj);
    WRITE_PROPORTION_RECT_FIELD_TO_JSON(this->hotkeyRects, f8, hotkeyRectsObj);
    WRITE_PROPORTION_RECT_FIELD_TO_JSON(this->hotkeyRects, f9, hotkeyRectsObj);
    WRITE_PROPORTION_RECT_FIELD_TO_JSON(this->hotkeyRects, f10, hotkeyRectsObj);
    WRITE_PROPORTION_RECT_FIELD_TO_JSON(this->hotkeyRects, f11, hotkeyRectsObj);
    WRITE_PROPORTION_RECT_FIELD_TO_JSON(this->hotkeyRects, f12, hotkeyRectsObj);
    j["hotkeyRects"] = hotkeyRectsObj;

    nlohmann::json hpMpBarInfoObj;
    WRITE_PROPORTION_POS_FIELD_TO_JSON(this->hpMpBarInfo, samplingPos, hotkeyRectsObj);
    WRITE_RGB_COLOR_FIELD_TO_JSON(this->hpMpBarInfo, baseColor, hotkeyRectsObj);
    WRITE_RGB_COLOR_FIELD_TO_JSON(this->hpMpBarInfo, hpColor, hotkeyRectsObj);
    WRITE_RGB_COLOR_FIELD_TO_JSON(this->hpMpBarInfo, mpColor, hotkeyRectsObj);
    j["hpMpBarInfo"] = hpMpBarInfoObj;

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
