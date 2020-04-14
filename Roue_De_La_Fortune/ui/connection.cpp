#include "ui/connection.h"
#include "ui_connection.h"
#include <QMessageBox>
#include "entity/localplayer.h"
#include "event/eventmanager.h"
#include "ui/servernavigator.h"
#include "ui/inscription.h"
#include <iostream>

Connection::Connection(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Connection)
{
    ui->setupUi(this);

    EventManager::getInstance()->addListener(EventManager::EVENT::CONNEXION_SUCCESS, [&](void*){        
        emit showServerNavigator();
    });

    EventManager::getInstance()->addListener(EventManager::EVENT::CONNEXION_FAILURE, [&](void* msg){        
        emit displayError(QString::fromStdString(*static_cast<std::string*>(msg)));
    });

    connect(this, SIGNAL(displayError(QString)), this, SLOT(connection_error(QString)));
    connect(this, SIGNAL(showServerNavigator()), this, SLOT(connection_success()));
}

Connection::~Connection()
{
    delete ui;
}

void Connection::on_connectionButton_clicked()
{
    LocalPlayer::setName(ui->lineEmail->text().toStdString());
    LocalPlayer::getInstance()->setPassword(ui->linePsw->text().toStdString());
    LocalPlayer::getInstance()->login();
}

void Connection::connection_error(QString msg)
{
    QMessageBox msgBox;
    msgBox.setWindowTitle("Connection");
    msgBox.setText("Erreur");
    msgBox.setInformativeText(msg);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.exec();
}

void Connection::connection_success()
{
    ServerNavigator* serverNavigator = new ServerNavigator();
    serverNavigator->show();
    this->close();
}

void Connection::on_inscriptionButton_clicked()
{
    Inscription* inscription = new Inscription();
    inscription->show();
    this->close();
}