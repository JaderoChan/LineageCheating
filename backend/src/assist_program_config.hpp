#pragma once

#include <cstdint>

#include <nlohmann/json.hpp>

struct AssistProgramConfig
{
    static AssistProgramConfig fromJson(const nlohmann::json& json);
    static AssistProgramConfig fromFile(const std::string& filepath);

    nlohmann::json toJson() const;
    void toFile(const std::string& filepath) const;

    // 有关时间的字段均使用毫秒为单位。

    bool enableBackhomeOnFootmanHpLow = true;   ///< 是否启用仆从机血量过低时回城。

    uint32_t treatTimeInterval = 200;           ///< 使用治疗药水的最小间隔。
    uint32_t frameGetterTimeout = 10 * 1000;    ///< 帧获取函数的超时时间，如果超时未获得帧将退出工作线程。默认为 `10` 秒。
    uint32_t cps = 10;                          ///< 鼠标每秒点击数。

    int masterTreatKey = 0x75;                  ///< 主人治疗药水键，默认为 `VK_F6`。
    int footmanTreatKey = 0x74;                 ///< 仆从治疗药水键，默认为 `VK_F5`。
    int backHomeKey = 0x76;                     ///< 回城键，默认为 `VK_F7`。

    double colorConfidence = 0.90;              ///< 血条/蓝条指定位置的颜色置信度，用于判断血条的指定位置是否为底色。
    double masterTreatHpThresold = 0.302752;    ///< 主人机治疗阈值。
    double footmanTreatHpThresold = 0.302752;   ///< 仆从机治疗阈值。
    double footmanBackHomeHpThreshold = 0.705369;   ///< 当血条降低到此阈值时将仆从回城，并退出工作线程，以防游戏人物意外死亡。

    bool outputLog = true;                      ///< 启用日志。
    bool showDebugWindow = false;               ///< 是否显示Debug窗口，可显示处理后的游戏帧。
    bool limitDebugWindowSize = true;           ///< 限制Debug窗口的大小。
    int debugWindowMaxWidth = 1080;
    int debugWindowMaxHeight = 1080;
};
