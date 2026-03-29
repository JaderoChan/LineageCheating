#include "settings.h"

#include <qsettings.h>

Settings loadSettings()
{
    Settings settings;
    QSettings qsettings;

    settings.language = qsettings.value("Language", getCurrentSystemLang()).value<Language>();
    settings.gameDataFilepath = qsettings.value("GameDataFilepath", "").toString();
    settings.assistProgramConfigFilepath = qsettings.value("AssistProgramConfigFilepath", "").toString();

    return settings;
}

void saveSettings(const Settings& settings)
{
    QSettings qsettings;

    qsettings.setValue("Language", settings.language);
    qsettings.setValue("GameDataFilepath", settings.gameDataFilepath);
    qsettings.setValue("AssistProgramConfigFilepath", settings.assistProgramConfigFilepath);
}
