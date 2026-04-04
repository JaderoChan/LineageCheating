#include "hotkey_manager.h"

#include <utils/debug_output.h>

HotkeyManager::HotkeyManager(QObject* parent)
    : QObject(parent), ghm_(gbhk::RegisterGlobalHotkeyManager::getInstance())
{
    int rc = ghm_.run();
    if (rc != gbhk::RC_SUCCESS)
        debugOut(qCritical(),
            "[Hotkey Manager] Failed to run the Global Hotkey Manager. Error message: %1.",
            gbhk::getReturnCodeMessage(rc).c_str());
}

HotkeyManager::~HotkeyManager()
{
    int rc = ghm_.stop();
    if (rc != gbhk::RC_SUCCESS)
        debugOut(qCritical(),
            "[Hotkey Manager] Failed to stop the Global Hotkey Manager. Error message: %1.",
            gbhk::getReturnCodeMessage(rc).c_str());
}

void HotkeyManager::setRunHotkey(const gbhk::KeyCombination& hotkey)
{
    setHotkey(true, hotkey);
}

void HotkeyManager::setStopHotkey(const gbhk::KeyCombination& hotkey)
{
    setHotkey(false, hotkey);
}

void HotkeyManager::setHotkey(bool isRunHotkey, const gbhk::KeyCombination& newHotkey)
{
    gbhk::KeyCombination& oldHotkey = (isRunHotkey ? runHotkey_ : stopHotkey_);

    if (ghm_.isHotkeyRegistered(oldHotkey))
    {
        if (newHotkey.isValid())
        {
            int rc = ghm_.replaceHotkey(oldHotkey, newHotkey);
            if (rc != gbhk::RC_SUCCESS)
                debugOut(qCritical(),
                    "[Hotkey Manager] Failed to replace hotkey from '%1' to '%2'. Error message: %3.",
                    oldHotkey.toString().c_str(),
                    newHotkey.toString().c_str(),
                    gbhk::getReturnCodeMessage(rc).c_str());
        }
        else
        {
            int rc = ghm_.unregisterHotkey(oldHotkey);
            if (rc != gbhk::RC_SUCCESS)
                debugOut(qCritical(),
                    "[Hotkey Manager] Failed to unregister hotkey '%1'. Error message: %2.",
                    oldHotkey.toString().c_str(),
                    gbhk::getReturnCodeMessage(rc).c_str());
        }
    }
    else
    {
        if (newHotkey.isValid())
        {
            int rc = ghm_.registerHotkey(newHotkey, [=]()
            { emit (isRunHotkey ? runHotkeyTriggered() : stopHotkeyTriggered()); });
            if (rc != gbhk::RC_SUCCESS)
                debugOut(qCritical(),
                    "[Hotkey Manager] Failed to register hotkey '%1'. Error message: %2.",
                    newHotkey.toString().c_str(),
                    gbhk::getReturnCodeMessage(rc).c_str());
        }
    }

    oldHotkey = newHotkey;
}
