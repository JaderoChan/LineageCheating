#include "system_tray.h"

#include <easy_translate.hpp>

#include "logo_icon.h"

SystemTray::SystemTray(QObject* parent)
    : QSystemTrayIcon(parent), menu_(new QMenu())
{
    setIcon(getLogoIcon());

    connectStateAction_ = new QAction(this);
    connectAction_ = new QAction(this);
    disconnectAction_ = new QAction(this);
    settingsAction_ = new QAction(this);
    exitAction_ = new QAction(this);

    menu_.addActions({
        connectStateAction_, connectAction_, disconnectAction_, settingsAction_, exitAction_
    });
    setContextMenu(&menu_);

connect(this, &QSystemTrayIcon::activated, this, &SystemTray::onActivated);

    connect(connectAction_, &QAction::triggered, this, [this]() { emit connectActionTriggered(); });
    connect(disconnectAction_, &QAction::triggered, this, [this]() { emit disconnectActionTriggered(); });
    connect(settingsAction_, &QAction::triggered, this, [this]() { emit settingsActionTriggered(); });
    connect(exitAction_, &QAction::triggered, this, [this]() { emit exitActionTriggered(); });

    show();
    updateText();
    updateConnectState(false);
}

void SystemTray::updateText()
{
    setToolTip(EASYTR("Lineage Cheating Remote Control"));

    connectStateAction_->setText(EASYTR(isConnected_ ? "Connected" : "Unconnected"));
    connectAction_->setText(EASYTR("Connect"));
    disconnectAction_->setText(EASYTR("Disconnect"));
    settingsAction_->setText(EASYTR("Settings"));
    exitAction_->setText(EASYTR("Exit"));
}

void SystemTray::updateConnectState(bool isConnected)
{
    isConnected_ = isConnected;
    connectStateAction_->setText(EASYTR(isConnected_ ? "Connected" : "Unconnected"));
    connectStateAction_->setIcon(isConnected_ ? QIcon(":/icons/green_circle.png") : QIcon(":/icons/red_circle.png"));
}

void SystemTray::onActivated(ActivationReason reason)
{
    switch (reason)
    {
        case Trigger:   // Fallthrough
        case Context:
            contextMenu()->popup(QCursor::pos());
            break;
        default:
            break;
    }
}
