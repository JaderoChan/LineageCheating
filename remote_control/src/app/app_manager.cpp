#include "app_manager.h"

#include <nlohmann/json.hpp>

#include <utils/debug_output.h>

#include "settings.h"
#include "settings_dialog.h"

AppManager::AppManager(QObject* parent)
    : QObject(parent),
    hotkeyMgr_(new HotkeyManager(this)),
    systemTray_(new SystemTray(this)),
    socket_(new QWebSocket())
{
    socket_->setParent(this);

    auto settings = loadSettings();
    hotkeyMgr_->setRunHotkey(settings.runHotkey);
    hotkeyMgr_->setStopHotkey(settings.stopHotkey);

    QObject::connect(hotkeyMgr_, &HotkeyManager::runHotkeyTriggered, this, [this]() { onHotkeyTriggered(true); });
    QObject::connect(hotkeyMgr_, &HotkeyManager::runHotkeyTriggered, this, [this]() { onHotkeyTriggered(false); });

    QObject::connect(systemTray_, &SystemTray::connectActionTriggered, this, &AppManager::connect);
    QObject::connect(systemTray_, &SystemTray::disconnectActionTriggered, this, &AppManager::disconnect);
    QObject::connect(systemTray_, &SystemTray::settingsActionTriggered, this, &AppManager::onSettingsActionTriggered);
    QObject::connect(systemTray_, &SystemTray::exitActionTriggered, this, [=]() { qApp->exit(); });

    QObject::connect(socket_, &QWebSocket::connected, this, &AppManager::onConnected);
    QObject::connect(socket_, &QWebSocket::disconnected, this, &AppManager::onDisconnected);
}

AppManager::~AppManager()
{
    disconnect();
}

void AppManager::connect()
{
    disconnect();
    auto settings = loadSettings();
    socket_->open(QUrl(settings.serverUrl));
}

void AppManager::disconnect()
{
    if (isConnected())
        socket_->close();
}

bool AppManager::isConnected() const
{
    return socket_ && (socket_->state() == QAbstractSocket::SocketState::ConnectedState);
}

void AppManager::sendTextMessage(const QString& text)
{
    if (isConnected())
    {
        debugOut(qInfo(), "Send message: %1.", text);
        socket_->sendTextMessage(text);
    }
    else
    {
        debugOut(qWarning(), "The socket is unconnected.");
    }
}

void AppManager::onConnected()
{
    debugOut(qInfo(), "Connected: %1.", socket_->peerAddress().toString());
    systemTray_->updateConnectState(true);
}

void AppManager::onDisconnected()
{
    debugOut(qInfo(), "Disonnected: %1.", socket_->peerAddress().toString());
    systemTray_->updateConnectState(false);
}

void AppManager::onHotkeyTriggered(bool isRunHotkey)
{
    debugOut(qInfo(), "The hotkey is triggered: %1.", (isRunHotkey ? "Run Hotkey" : "Stop Hotkey"));

    auto settings = loadSettings();

    nlohmann::json j;
    j["index"] = settings.index;
    j["flag"] = isRunHotkey ? 1 : 2;

    QString jsonStr = QString::fromStdString(j.dump());
    sendTextMessage(jsonStr);
}

void AppManager::onSettingsActionTriggered()
{
    auto settings = loadSettings();
    SettingsDialog dlg(settings);

    bool isConfirm;
    auto newSettings = dlg.execForSettings(isConfirm);

    if (isConfirm)
    {
        if (newSettings.language != settings.language)
            systemTray_->updateText();
        saveSettings(newSettings);
    }
}
