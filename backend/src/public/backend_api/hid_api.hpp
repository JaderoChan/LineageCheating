#pragma once

#include <windows.h>

namespace hid
{

using HID = HANDLE;

HID openHID(int port);

HID openHID(int vid, int pid);

int closeHID(HID hid);

int pressKey(HID hid, int keyCode);

int releaseKey(HID hid, int keyCode);

int clickKey(HID hid, int keyCode, int repeatCount = 1);

int releaseAllKey(HID hid);

/**
 * @brief Query the status of the specified key.
 * @return 0: Release status. 1: Pressed status. -1: Failed to get status.
 */
int getKeyState(HID hid, int keyCode);

/// @param button 1: Left button. 2: Right button. 3: Middle button.
int pressMouseButton(HID hid, int button);

int releaseMouseButton(HID hid, int button);

int clickMouseButton(HID hid, int button, int repeatCount = 1);

int relaseAllMouseButton(HID hid);

/// @return 0: Release status. 1: Pressed status. -1: Failed to get status.
int getMouseButtonState(HID hid, int button);

int setResolution(HID hid, int x, int y);

int moveMouseTo(HID hid, int x, int y);

int relativeMoveMouse(HID hid, int dx, int dy);

int getMousePos(HID hid, int& x, int& y);

/// @param delta The amount of scroll wheel movement. Positive numbers scroll up, negative numbers scroll down.
int scrollMouseWheel(HID hid, int delta);

} // namespace hid
