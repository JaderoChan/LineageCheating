#include "settings.h"

#include <qsettings.h>

Settings loadSettings()
{
    Settings settings;
    QSettings qsettings;

    settings.language = qsettings.value("Language", getCurrentSystemLang()).value<Language>();
    settings.assistProgramWorkConfigs = qsettings.value("AssistProgramWorkConfigs", QVector<QString>()).toStringList();
    settings.serverPort = qsettings.value("ServerPort", 8080).toInt();

    return settings;
}

void saveSettings(const Settings& settings)
{
    QSettings qsettings;

    qsettings.setValue("Language", settings.language);
    qsettings.setValue("AssistProgramWorkConfigs", settings.assistProgramWorkConfigs);
    qsettings.setValue("ServerPort", settings.serverPort);
}
