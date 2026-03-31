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

struct WorkConfig
{
    WorkConfig() = default;

    static WorkConfig fromJson(const nlohmann::json& json);
    static WorkConfig fromFile(const QString& filepath);

    nlohmann::json toJson() const;
    void toFile(const QString& filepath) const;

    struct HIDInfo
    {
        int vid = 0;
        int pid = 0;
    };

    QString name;
    WorkType type;

    QString masterNdiSourceName;
    QString footmanNdiSourceName;

    HIDInfo masterHidInfo;
    HIDInfo footmanHidInfo;

    QString gameDataPath;
    QString configPath;
};
