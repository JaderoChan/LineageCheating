#pragma once

#include <qobject.h>

#include <global_hotkey/global_hotkey.hpp>

class HotkeyManager : public QObject
{
    Q_OBJECT

public:
    explicit HotkeyManager(QObject* parent = nullptr);
    ~HotkeyManager();

    void setRunHotkey(const gbhk::KeyCombination& hotkey);
    void setStopHotkey(const gbhk::KeyCombination& hotkey);

signals:
    void runHotkeyTriggered();
    void stopHotkeyTriggered();

protected:
    void setHotkey(bool isRunHotkey, const gbhk::KeyCombination& newHotkey);

private:
    gbhk::GlobalHotkeyManager& ghm_;
    gbhk::KeyCombination runHotkey_;
    gbhk::KeyCombination stopHotkey_;
};
