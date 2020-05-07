#include "inscription.h"
#include "ui_inscription.h"
#include "entity/localplayer.h"
#include "protocol/protocolhandler.h"
#include "event/eventmanager.h"
#include "ui/connection.h"
#include <QMessageBox>
#include <QRegExpValidator>

Inscription::Inscription(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Inscription)
{
    ui->setupUi(this);
    this->setFixedSize(this->size());

    EventManager::getInstance()->addListener(EventManager::INSCRIPTION_FAILURE, [&](void* msg){
        emit displayInscriptionError(QString::fromStdString(*static_cast<std::string*>(msg)));
    });

    EventManager::getInstance()->addListener(EventManager::INSCRIPTION_SUCCESS, [&](void*){
        emit showConnectionForm();
    });


    connect(this, SIGNAL(displayInscriptionError(QString)), this, SLOT(inscriptionError(QString)));
    connect(this, SIGNAL(showConnectionForm()), this, SLOT(inscriptionSuccessful()));

    ui->pseudoLine->setValidator(new QRegExpValidator(QRegExp("[A-Za-z0-9_ ]{0,20}"), this));
    ui->passwordLine->setValidator(new QRegExpValidator(QRegExp("[A-Za-z0-9_ ]{0,20}"), this));
}

Inscription::~Inscription()
{
    delete ui;
}

void Inscription::on_buttonInscription_clicked()
{
    ProtocolHandler protocolHandler;
    std::string username = ui->pseudoLine->text().toStdString();
    std::string password = ui->passwordLine->text().toStdString();

    LocalPlayer::getInstance()->sendMessage(protocolHandler.getInscriptionProtocol(username, password));
}

void Inscription::inscriptionError(QString msg)
{
    QMessageBox msgBox;
    msgBox.setWindowTitle("Inscription");
    msgBox.setText("Erreur");
    msgBox.setInformativeText(msg);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.exec();
}

void Inscription::inscriptionSuccessful()
{
    QMessageBox msgBox;
    msgBox.setWindowTitle("Inscription");
    msgBox.setText("OK");
    msgBox.setInformativeText("vous êtes désormais inscrit.");
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.setIcon(QMessageBox::Icon::Information);
    msgBox.exec();

    Connection* connection = new Connection();
    connection->show();
    this->close();
}

void Inscription::on_buttonCancel_clicked()
{
    Connection* connection = new Connection();
    connection->show();
    this->close();
}
