#pragma once

#include <qstring.h>
#include <qlist.h>

#include "language.h"

struct Settings
{
    Language language;
    QList<QString> assistProgramWorkConfigs;
    int serverPort = 8080;
};

Settings loadSettings();

void saveSettings(const Settings& settings);
