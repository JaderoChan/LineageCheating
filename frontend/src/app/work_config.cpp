#include "work_config.h"

#include <utils/file_io.h>

WorkConfig WorkConfig::fromJson(const nlohmann::json& json)
{
    WorkConfig result;

    result.name = QString::fromStdString(json.at("name"));
    result.type = static_cast<WorkType>(json.at("type"));

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

    j["name"] = name.toStdString();
    j["type"] = type;

    j["gameDataPath"] = gameDataPath.toStdString();
    j["configPath"] = configPath.toStdString();

    return j;
}

void WorkConfig::toFile(const QString& filepath) const
{
    std::string jsonStr = toJson().dump(4);
    writeFileContent(filepath, QString::fromStdString(jsonStr));
}

AssistProgramWorkConfig AssistProgramWorkConfig::fromJson(const nlohmann::json& json)
{
    AssistProgramWorkConfig result;

    result.masterNdiSourceName = QString::fromStdString(json.at("masterNdiSourceName"));
    result.footmanNdiSourceName = QString::fromStdString(json.at("footmanNdiSourceName"));

    nlohmann::json footmanHidInfoObj = json.at("footmanHidInfo");
    result.footmanHidInfo.vid = footmanHidInfoObj["vid"];
    result.footmanHidInfo.pid = footmanHidInfoObj["pid"];

    result.config = AssistProgramConfig::fromJson(json["config"]);

    return result;
}

AssistProgramWorkConfig AssistProgramWorkConfig::fromFile(const QString& filepath)
{
    QString jsonStr = readFileContent(filepath);
    nlohmann::json j = nlohmann::json::parse(jsonStr.toStdString(), nullptr, true, true);
    return AssistProgramWorkConfig::fromJson(j);
}

nlohmann::json AssistProgramWorkConfig::toJson() const
{
    nlohmann::json j;

    j["masterNdiSourceName"] = masterNdiSourceName.toStdString();
    j["footmanNdiSourceName"] = footmanNdiSourceName.toStdString();

    nlohmann::json footmanHidInfoObj;
    footmanHidInfoObj["vid"] = footmanHidInfo.vid;
    footmanHidInfoObj["pid"] = footmanHidInfo.pid;
    j["footmanHidInfo"] = footmanHidInfoObj;

    j["config"] = config.toJson();

    return j;
}

void AssistProgramWorkConfig::toFile(const QString& filepath) const
{
    std::string jsonStr = toJson().dump(4);
    writeFileContent(filepath, QString::fromStdString(jsonStr));
}
