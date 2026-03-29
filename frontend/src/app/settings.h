#pragma once

#include <qstring.h>

#include "language.h"

struct Settings
{
    Language language;
    QString gameDataFilepath;
    QString assistProgramConfigFilepath;
};

Settings loadSettings();

void saveSettings(const Settings& settings);
