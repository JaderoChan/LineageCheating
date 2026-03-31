#pragma once

#include <qstring.h>

#include <nlohmann/json.hpp>

#include <game_data.hpp>
#include <assist_program_config.hpp>

enum WorkType
{
    WT_ASSIST,
    WT_AUTO_SHOOTING
};

struct HIDInfo
{
    int vid = 0;
    int pid = 0;
};

struct WorkConfig
{
    WorkConfig() = default;

    static WorkConfig fromJson(const nlohmann::json& json);
    static WorkConfig fromFile(const QString& filepath);

    nlohmann::json toJson() const;
    void toFile(const QString& filepath) const;

    QString name;
    WorkType type;

    QString gameDataPath;
    QString configPath;
};

struct AssistProgramWorkConfig
{
    AssistProgramWorkConfig() = default;

    static AssistProgramWorkConfig fromJson(const nlohmann::json& json);
    static AssistProgramWorkConfig fromFile(const QString& filepath);

    nlohmann::json toJson() const;
    void toFile(const QString& filepath) const;

    QString masterNdiSourceName;
    QString footmanNdiSourceName;

    HIDInfo footmanHidInfo;

    AssistProgramConfig config;
};
