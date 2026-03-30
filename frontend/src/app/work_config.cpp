#include "work_config.h"

#include <utils/file_io.h>

WorkConfig WorkConfig::fromJson(const nlohmann::json& json)
{
    WorkConfig result;

    result.type = static_cast<WorkType>(json.at("type"));

    result.masterNdiSourceName = QString::fromStdString(json.at("masterNdiSourceName"));
    result.footmanNdiSourceName = QString::fromStdString(json.at("footmanNdiSourceName"));

    nlohmann::json masterHidInfoObj = json.at("masterHidInfo");
    result.masterHidInfo.vid = masterHidInfoObj["vid"];
    result.masterHidInfo.pid = masterHidInfoObj["pid"];

    nlohmann::json footmanHidInfoObj = json.at("footmanHidInfo");
    result.footmanHidInfo.vid = footmanHidInfoObj["vid"];
    result.footmanHidInfo.pid = footmanHidInfoObj["pid"];

    result.gameDataPath = QString::fromStdString(json.at("gameDataPath"));
    result.configPath = QString::fromStdString(json.at("configPath"));

    return result;
}

WorkConfig WorkConfig::fromFile(const QString& filepath)
{
    QString jsonStr = readFileContent(filepath);
    nlohmann::json j = nlohmann::json::parse(jsonStr.toStdString(), nullptr, true, true);
    return WorkConfig::fromJson(j);
}

nlohmann::json WorkConfig::toJson() const
{
    nlohmann::json j;

    j["type"] = type;

    j["masterNdiSourceName"] = masterNdiSourceName.toStdString();
    j["footmanNdiSourceName"] = footmanNdiSourceName.toStdString();

    nlohmann::json masterHidInfoObj;
    masterHidInfoObj["vid"] = masterHidInfo.vid;
    masterHidInfoObj["pid"] = masterHidInfo.pid;
    j["masterHidInfo"] = masterHidInfoObj;

    nlohmann::json footmanHidInfoObj;
    footmanHidInfoObj["vid"] = footmanHidInfo.vid;
    footmanHidInfoObj["pid"] = footmanHidInfo.pid;
    j["footmanHidInfo"] = footmanHidInfoObj;

    j["gameDataPath"] = gameDataPath.toStdString();
    j["configPath"] = configPath.toStdString();

    return j;
}

void WorkConfig::toFile(const QString& filepath) const
{
    std::string jsonStr = toJson().dump(4);
    writeFileContent(filepath, QString::fromStdString(jsonStr));
}
