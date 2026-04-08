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

    connect(hotkeyMgr_, &HotkeyManager::runHotkeyTriggered, this, [this]() { onHotkeyTriggered(true); });
    connect(hotkeyMgr_, &HotkeyManager::stopHotkeyTriggered, this, [this]() { onHotkeyTriggered(false); });

    connect(systemTray_, &SystemTray::connectActionTriggered, this, &AppManager::connectToServer);
    connect(systemTray_, &SystemTray::disconnectActionTriggered, this, &AppManager::disconnectFromServer);
    connect(systemTray_, &SystemTray::settingsActionTriggered, this, &AppManager::onSettingsActionTriggered);
    connect(systemTray_, &SystemTray::exitActionTriggered, this, [=]() { qApp->exit(); });

    connect(socket_, &QWebSocket::connected, this, &AppManager::onConnected);
    connect(socket_, &QWebSocket::disconnected, this, &AppManager::onDisconnected);
}

AppManager::~AppManager()
{
    disconnect();
}

void AppManager::connectToServer()
{
    disconnectFromServer();
    auto settings = loadSettings();

    QString urlStr = settings.serverUrl;
    if (!urlStr.startsWith("ws://") && !urlStr.startsWith("wss://"))
        urlStr = "ws://" + urlStr;

    debugOut(qInfo(), "Server URL: %1.", urlStr);
    socket_->open(urlStr);
}

void AppManager::disconnectFromServer()
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
    debugOut(qInfo(), "Disconnected.");
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
        if (newSettings.runHotkey != settings.runHotkey)
            hotkeyMgr_->setRunHotkey(newSettings.runHotkey);
        if (newSettings.stopHotkey != settings.stopHotkey)
            hotkeyMgr_->setStopHotkey(newSettings.stopHotkey);
        saveSettings(newSettings);
    }
}
