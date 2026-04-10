#include "settings.h"

#include <qsettings.h>

Settings loadSettings()
{
    Settings settings;
    QSettings qsettings;

    settings.language = qsettings.value("Language", getCurrentSystemLang()).value<Language>();
    settings.index = qsettings.value("Index", 0).toInt();

    auto runHotkeyStr = qsettings.value("RunHotkey", "Alt+F11").toString().toStdString();
    settings.runHotkey = gbhk::KeyCombination::fromString(runHotkeyStr);

    auto stopHotkeyStr = qsettings.value("StopHotkey", "Alt+F12").toString().toStdString();
    settings.stopHotkey = gbhk::KeyCombination::fromString(stopHotkeyStr);

    settings.serverUrl = qsettings.value("ServerURL", "").toString();

    return settings;
}

void saveSettings(const Settings& settings)
{
    QSettings qsettings;

    qsettings.setValue("Language", settings.language);
    qsettings.setValue("Index", settings.index);
    qsettings.setValue("RunHotkey", settings.runHotkey.toString().c_str());
    qsettings.setValue("StopHotkey", settings.stopHotkey.toString().c_str());
    qsettings.setValue("ServerURL", settings.serverUrl);
}
