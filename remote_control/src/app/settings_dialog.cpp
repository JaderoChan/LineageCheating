#include "settings_dialog.h"

#include <qvalidator.h>

SettingsDialog::SettingsDialog(const Settings& settings, QWidget* parent)
    : TrDialog(parent), settings_(settings)
{
    ui.setupUi(this);

    QIntValidator* validator = new QIntValidator(0, 128, this);
    ui.indexLineEdit->setValidator(validator);

    ui.indexLineEdit->setText(QString::number(settings_.index));
    ui.runHotkeyInputer->setKeyCombination(QKeySequence::fromString(settings_.runHotkey.toString().c_str()));
    ui.stopHotkeyInputer->setKeyCombination(QKeySequence::fromString(settings_.stopHotkey.toString().c_str()));
    ui.serverUrlLineEdit->setText(settings_.serverUrl);

    connect(ui.confirmButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(ui.cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    updateText();
}

Settings SettingsDialog::execForSettings(bool& isConfirm)
{
    int result = exec();
    if (result == QDialog::Accepted)
    {
        settings_.index = ui.indexLineEdit->text().toInt();
        auto runHotkeyStr = QKeySequence(ui.runHotkeyInputer->keyCombination()).toString();
        settings_.runHotkey = gbhk::KeyCombination::fromString(runHotkeyStr.toStdString());
        auto stopHotkeyStr = QKeySequence(ui.stopHotkeyInputer->keyCombination()).toString();
        settings_.stopHotkey = gbhk::KeyCombination::fromString(stopHotkeyStr.toStdString());
        settings_.serverUrl = ui.serverUrlLineEdit->text();

        isConfirm = true;
        return settings_;
    }
    else
    {
        isConfirm = false;
        return Settings();
    }
}

void SettingsDialog::updateText()
{
    setWindowTitle(EASYTR("Settings"));

    ui.indexTextLabel->setText(EASYTR("Work Index"));
    ui.runHotkeyTextLabel->setText(EASYTR("Run Hotkey"));
    ui.stopHotkeyTextLabel->setText(EASYTR("Stop Hotkey"));
    ui.serverUrlTextLabel->setText(EASYTR("Server URL"));
    ui.confirmButton->setText(EASYTR("Confirm"));
    ui.cancelButton->setText(EASYTR("Cancel"));
}
