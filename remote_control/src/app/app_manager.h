#pragma once

#include <qobject.h>
#include <qwebsocket.h>

#include "hotkey_manager.h"
#include "system_tray.h"

class AppManager : public QObject
{
    Q_OBJECT

public:
    explicit AppManager(QObject* parent = nullptr);
    ~AppManager();

    void connectToServer();
    void disconnectFromServer();
    bool isConnected() const;

    void sendTextMessage(const QString& text);

protected:
    void onConnected();
    void onDisconnected();

    void onHotkeyTriggered(bool isRunHotkey);
    void onSettingsActionTriggered();

private:
    HotkeyManager* hotkeyMgr_ = nullptr;
    SystemTray* systemTray_ = nullptr;
    QWebSocket* socket_ = nullptr;
};
