#pragma once

#include "data_struct_rw.hpp"
#include "proportion_pos.hpp"
#include "proportion_rect.hpp"
#include "rgb_color.hpp"

struct GameData
{
    static GameData fromJson(const nlohmann::json& json);
    static GameData fromFile(const std::string& filepath);

    nlohmann::json toJson() const;
    void toFile(const std::string& filepath) const;

    // 以下字段值均建立在游戏画面为包含黑边的最大化窗口的基础上。（1920*1080）
    ProportionRect mainRect;        ///< 游戏画面有效区域，不包含黑边。
    ProportionRect safeRect;        ///< 游戏画面安全区域，不包含下方控制栏区域、Buff区域、弹出式菜单区域。
    ProportionRect playerRect;      ///< 玩家角色区域。
    ProportionRect bottomRect;      ///< 下方控制栏区域。
    ProportionRect buffRect;        ///< Buff区域。
    ProportionRect popupMenuRect;   ///< 弹出式设置菜单区域。
    ProportionRect backpackRect;    ///< 背包菜单区域。
    ProportionRect hpRect;          ///< 血条区域。
    ProportionRect mpRect;          ///< 蓝条区域。
    ProportionRect chatRect;        ///< 聊天栏区域。

    struct
    {
        ProportionRect f5;
        ProportionRect f6;
        ProportionRect f7;
        ProportionRect f8;
        ProportionRect f9;
        ProportionRect f10;
        ProportionRect f11;
        ProportionRect f12;
    } hotkeyRects;

    struct
    {
        ProportionPos samplingPos;  ///< 颜色采样点。（相对于血条区域）
        RgbColor baseColor;         ///< 底色。
        RgbColor hpColor;           ///< 血条色。
        RgbColor mpColor;           ///< 蓝条色。
    } hpMpBarInfo;
};
