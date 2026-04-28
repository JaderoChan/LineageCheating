#include "assist_program_config.hpp"

#include <fstream>

#define READ_FIELD_FROM_JSON(struct, field, jsonObj, defaultValue) (struct).field = (jsonObj).value(#field, (defaultValue))
#define WRITE_FIELD_TO_JSON(struct, field, jsonObj) (jsonObj)[#field] = (struct).field

AssistProgramConfig AssistProgramConfig::fromJson(const nlohmann::json& json)
{
    AssistProgramConfig result;

    READ_FIELD_FROM_JSON(result, enableBackhomeOnFootmanHpLow, json, true);

    READ_FIELD_FROM_JSON(result, treatTimeInterval, json, 200);
    READ_FIELD_FROM_JSON(result, frameGetterTimeout, json, 10 * 1000);
    READ_FIELD_FROM_JSON(result, cps, json, 10);

    READ_FIELD_FROM_JSON(result, masterTreatKey, json, 0x75);
    READ_FIELD_FROM_JSON(result, footmanTreatKey, json, 0x74);
    READ_FIELD_FROM_JSON(result, backHomeKey, json, 0x76);

    READ_FIELD_FROM_JSON(result, colorConfidence, json, 0.86);
    READ_FIELD_FROM_JSON(result, masterTreatHpThresold, json, 0.302752);
    READ_FIELD_FROM_JSON(result, footmanTreatHpThresold, json, 0.302752);
    READ_FIELD_FROM_JSON(result, footmanBackHomeHpThreshold, json, 0.705369);

    READ_FIELD_FROM_JSON(result, outputLog, json, true);
    READ_FIELD_FROM_JSON(result, showDebugWindow, json, false);
    READ_FIELD_FROM_JSON(result, limitDebugWindowSize, json, true);
    READ_FIELD_FROM_JSON(result, debugWindowMaxWidth, json, 1080);
    READ_FIELD_FROM_JSON(result, debugWindowMaxHeight, json, 1080);

    return result;
}

AssistProgramConfig AssistProgramConfig::fromFile(const std::string& filepath)
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

nlohmann::json AssistProgramConfig::toJson() const
{
    nlohmann::json j;

    WRITE_FIELD_TO_JSON(*this, enableBackhomeOnFootmanHpLow, j);

    WRITE_FIELD_TO_JSON(*this, treatTimeInterval, j);
    WRITE_FIELD_TO_JSON(*this, frameGetterTimeout, j);
    WRITE_FIELD_TO_JSON(*this, cps, j);

    WRITE_FIELD_TO_JSON(*this, masterTreatKey, j);
    WRITE_FIELD_TO_JSON(*this, footmanTreatKey, j);
    WRITE_FIELD_TO_JSON(*this, backHomeKey, j);

    WRITE_FIELD_TO_JSON(*this, colorConfidence, j);
    WRITE_FIELD_TO_JSON(*this, masterTreatHpThresold, j);
    WRITE_FIELD_TO_JSON(*this, footmanTreatHpThresold, j);
    WRITE_FIELD_TO_JSON(*this, footmanBackHomeHpThreshold, j);

    WRITE_FIELD_TO_JSON(*this, outputLog, j);
    WRITE_FIELD_TO_JSON(*this, showDebugWindow, j);
    WRITE_FIELD_TO_JSON(*this, limitDebugWindowSize, j);
    WRITE_FIELD_TO_JSON(*this, debugWindowMaxWidth, j);
    WRITE_FIELD_TO_JSON(*this, debugWindowMaxHeight, j);

    return j;
}

void AssistProgramConfig::toFile(const std::string& filepath) const
{
    std::string jsonStr = toJson().dump(4);

    std::ofstream ofs(filepath);
    if (!ofs.is_open())
        throw std::runtime_error("Failed to open file: " + filepath);

    ofs << jsonStr;
    ofs.close();
}
