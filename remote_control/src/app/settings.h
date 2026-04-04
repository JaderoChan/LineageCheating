#pragma once

#include <qstring.h>

#include <global_hotkey/key_combination.hpp>

#include "language.h"

struct Settings
{
    Language language;
    int index;
    gbhk::KeyCombination runHotkey;
    gbhk::KeyCombination stopHotkey;
    QString serverUrl;
};

Settings loadSettings();

void saveSettings(const Settings& settings);
