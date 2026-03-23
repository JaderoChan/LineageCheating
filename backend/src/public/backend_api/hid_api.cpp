#include "hid_api.hpp"

#include <private/msdk.h>

namespace hid
{

HID openHID(int port)
{
    return M_Open(port);
}

HID openHID(int vid, int pid)
{
    return M_Open_VidPid(vid, pid);
}

int closeHID(HID hid)
{
    return M_Close(hid);
}

int pressKey(HID hid, int keyCode)
{
    return M_KeyDown2(hid, keyCode);
}

int releaseKey(HID hid, int keyCode)
{
    return M_KeyUp2(hid, keyCode);
}

int clickKey(HID hid, int keyCode, int repeatCount)
{
    return M_KeyPress2(hid, keyCode, repeatCount);
}

int releaseAllKey(HID hid)
{
    return M_ReleaseAllKey(hid);
}

int getKeyState(HID hid, int keyCode)
{
    return M_KeyState2(hid, keyCode);
}

int pressMouseButton(HID hid, int button)
{
    switch (button)
    {
        case 1:     return M_LeftDown(hid);
        case 2:     return M_RightDown(hid);
        case 3:     return M_MiddleDown(hid);
        default:    return -1;
    }
}

int releaseMouseButton(HID hid, int button)
{
    switch (button)
    {
        case 1:     return M_LeftUp(hid);
        case 2:     return M_RightUp(hid);
        case 3:     return M_MiddleUp(hid);
        default:    return -1;
    }
}

int clickMouseButton(HID hid, int button, int repeatCount)
{
    switch (button)
    {
        case 1:     return M_LeftClick(hid, repeatCount);
        case 2:     return M_RightClick(hid, repeatCount);
        case 3:     return M_MiddleClick(hid, repeatCount);
        default:    return -1;
    }
}

int relaseAllMouseButton(HID hid)
{
    return M_ReleaseAllMouse(hid);
}

int getMouseButtonState(HID hid, int button)
{
    return M_MouseKeyState(hid, button);
}

int setResolution(HID hid, int x, int y)
{
    return M_ResolutionUsed(hid, x, y);;
}

int moveMouseTo(HID hid, int x, int y)
{
    return M_MoveTo3(hid, x, y);
}

int relativeMoveMouse(HID hid, int dx, int dy)
{
    return M_MoveR(hid, dx, dy);
}

int getMousePos(HID hid, int& x, int& y)
{
    return M_GetCurrMousePos(hid, &x, &y);
}

int scrollMouseWheel(HID hid, int delta)
{
    return M_MouseWheel(hid, delta);
}

} // namespace hid
