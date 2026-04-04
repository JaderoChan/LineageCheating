#pragma once

#include <qobject.h>
#include <qaction.h>
#include <qmenu.h>
#include <qsystemtrayicon.h>

class SystemTray : public QSystemTrayIcon
{
    Q_OBJECT

public:
    explicit SystemTray(QObject* parent = nullptr);

    void updateText();
    void updateConnectState(bool isConnected);

signals:
    void connectActionTriggered();
    void disconnectActionTriggered();
    void settingsActionTriggered();
    void exitActionTriggered();

protected:
    void onActivated(ActivationReason reason);

private:
    bool isConnected_ = false;

    QMenu menu_;
    QAction* connectStateAction_ = nullptr;
    QAction* connectAction_ = nullptr;
    QAction* disconnectAction_ = nullptr;
    QAction* settingsAction_ = nullptr;
    QAction* exitAction_ = nullptr;
};
