#include "intro_dialog.h"

#include <QMessageBox>

#include <config.h>

#include "sha256.hpp"

IntroDialog::IntroDialog(QWidget* parent)
    : TrDialog(parent)
{
    ui.setupUi(this);

    connect(ui.confirmButton, &QPushButton::clicked, this, [this]()
    {
        if (checkPassword(ui.passwordLineEdit->text()))
            done(QDialog::Accepted);
        else
            QMessageBox::information(
                this,
                EASYTR("Warning"),
                EASYTR("Uncorrect password, please retry."),
                QMessageBox::Ok);
    });

    connect(ui.exitButton, &QPushButton::clicked, this, &TrDialog::reject);

    updateText();
}

bool IntroDialog::execForAccess()
{
    switch (exec())
    {
        case QDialog::Accepted:
            return true;
        default:
            return false;
    }
}

void IntroDialog::updateText()
{
    setWindowTitle(EASYTR("Lineage Cheating Tool"));

    ui.tipTextLabel->setText(EASYTR("Please input the password"));
    ui.passwordLineEdit->setPlaceholderText(EASYTR("Password"));
    ui.confirmButton->setText(EASYTR("Confirm"));
    ui.exitButton->setText(EASYTR("Exit"));
}

bool IntroDialog::checkPassword(const QString& password)
{
    return sha256(password.toStdString()) == PASSWORD_SHA256;
}
