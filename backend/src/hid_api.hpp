#pragma once

namespace hid
{

typedef void* HANDLE;

using HID = HANDLE;

HID openHID(int port);

HID openHID(int vid, int pid);

int closeHID(HID hid);

int pressKey(HID hid, int keyCode);

int releaseKey(HID hid, int keyCode);

int clickKey(HID hid, int keyCode, int repeatCount = 1);

int releaseAllKey(HID hid);

/**
 * @brief 查询指定按键的状态。
 * @return 0：释放状态；1：按下状态；-1：获取状态失败。
 */
int getKeyState(HID hid, int keyCode);

/// @param button 1：左键；2：右键；3：中键。
int pressMouseButton(HID hid, int button);

int releaseMouseButton(HID hid, int button);

int clickMouseButton(HID hid, int button, int repeatCount = 1);

int releaseAllMouseButton(HID hid);

/// @return 0：释放状态；1：按下状态；-1：获取状态失败。
int getMouseButtonState(HID hid, int button);

int setResolution(HID hid, int x, int y);

/// @attention 使用此函数之前必须首先通过 `setResolution()` 函数设置设备的屏幕分辨率。
int moveMouseTo(HID hid, int x, int y);

int relativeMoveMouse(HID hid, int dx, int dy);

int getMousePos(HID hid, int& x, int& y);

/// @param delta 滚轮移动的数量。正数向上滚动，负数向下滚动。
int scrollMouseWheel(HID hid, int delta);

} // namespace hid
