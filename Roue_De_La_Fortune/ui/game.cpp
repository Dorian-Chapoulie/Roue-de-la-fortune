#include "game.h"
#include "ui_game.h"
#include "event/eventmanager.h"
#include "protocol/protocolhandler.h"
#include "entity/localplayer.h"
#include <string>
#include <QThread>

#include <QMessageBox>
#include <iostream>

Game::Game(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Game)
{
    ui->setupUi(this);

    connect(this, SIGNAL(notifyNewPlayer(QString)), this, SLOT(addNewPlayer(QString)));
    connect(this, SIGNAL(notifyNewMessage(QString)), this, SLOT(addMessageToChat(QString)));
    connect(this, SIGNAL(notifyPlayerDisconnected(int)), this, SLOT(removePlayer(int)));

    ui->lineEditChat->setValidator(new QRegExpValidator(QRegExp("[A-Za-z0-9_ ]{0,50}"), this));

    const char consonnes[19] = {'b', 'c', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 'm', 'n', 'p', 'q', 'r', 's', 't', 'v', 'x', 'z'};
    const char voyelles[6] = {'a', 'e', 'i','o', 'u', 'y'};

    for(const auto& c : consonnes) {
        ui->comboBoxConsonne->addItem(QChar(c));
    }

    for(const auto& c : voyelles) {
        ui->comboBoxVoyelle->addItem(QChar(c));
    }



    LocalPlayer::getInstance()->login();

    EventManager::getInstance()->addListener(EventManager::EVENT::ASK_PSEUDO, [](void*){
        ProtocolHandler protocol;
        LocalPlayer::getInstance()->sendMessage(protocol.getPseudoProtocol(LocalPlayer::getInstance()->getName()));
    });

    EventManager::getInstance()->addListener(EventManager::EVENT::NEW_PLAYER, [&](void* pseudoAndId){
        std::string newPlayerNameAndId = *reinterpret_cast<std::string*>(pseudoAndId);
        emit notifyNewPlayer(QString::fromStdString(newPlayerNameAndId)); //direct cast to QString
    });

    EventManager::getInstance()->addListener(EventManager::EVENT::TCHAT, [&](void* data){
        std::string tchatData = *reinterpret_cast<std::string*>(data); //player-message
        emit notifyNewMessage(QString::fromStdString(tchatData));
    });

    EventManager::getInstance()->addListener(EventManager::PLAYER_DISCONNECT, [&](void* playerId){
        int id = std::stoi(*reinterpret_cast<std::string*>(playerId));
        emit notifyPlayerDisconnected(id);
    });

    scene = new QGraphicsScene(this);
    this->ui->graphicsView->setScene(scene);

}

Game::~Game()
{
    delete ui;
}


void Game::drawScene()
{
    scene->addLine(0, 0, 100, 100);

    QThread::create([&](){
        QThread::msleep(10);
        if(!false) { //stop
            while(!this->isActiveWindow()) QThread::msleep(10);
           // emit renderScene();
        }
    })->start();
}

void Game::addNewPlayer(QString data)
{
    std::string infos = data.toStdString();
    std::string name = infos.substr(0, infos.find('-'));
    std::string id = infos.substr(infos.find('-') + 1, infos.length());
    SOCKET s = std::stoi(id);

    for(Player* p : players) {
        if(p->getId() == s)  {
            return;
        }
    }


    players.push_back(new Player(name, s));

    switch(players.size()) {
        case 1:
            this->ui->labelPlayer1->setText(QString::fromStdString(name));
        break;

        case 2:
            this->ui->labelPlayer2->setText(QString::fromStdString(name));
        break;

        case 3:
            this->ui->labelPlayer3->setText(QString::fromStdString(name));
        break;
    }

    ui->textBrowserChat->append("<font color=\"Red\"><b>" + QString::fromStdString(name) + "</b> à rejoint la partie !");

}

void Game::addMessageToChat(QString msg)
{
    std::string pseudo = msg.toStdString();
    size_t pos = pseudo.find("-");
    pseudo = pseudo.substr(0, pos);
    std::string message = msg.toStdString().substr(pos + 1, msg.length() - pos);

    ui->textBrowserChat->append("<font color=\"Grey\"><b>" + QString::fromStdString(pseudo) + ":</b> "
                                + "<font color=\"Black\">" + QString::fromStdString(message));
}

void Game::removePlayer(int id)
{
    std::string pseudo = "";

    auto it = std::find_if(players.begin(), players.end(), [&](Player* p){
        return p->getId() == static_cast<SOCKET>(id);
    });

    pseudo = reinterpret_cast<Player*>(*it)->getName();
    players.erase(it);

    ui->textBrowserChat->append("<font color=\"Red\"><b>" + QString::fromStdString(pseudo) + " </b> "
                                + "<font color=\"Black\"> à quitté la partie");

    if(this->ui->labelPlayer1->text().toStdString() == pseudo) {
        this->ui->labelPlayer1->clear();
    }else if(this->ui->labelPlayer2->text().toStdString() == pseudo) {
        this->ui->labelPlayer2->clear();
    }if(this->ui->labelPlayer3->text().toStdString() == pseudo) {
        this->ui->labelPlayer3->clear();
    }
}

void Game::on_pushButtonChat_clicked()
{
    if(ui->lineEditChat->text().length() > 0) {
        ProtocolHandler protocolHandler;
        std::string msg = ui->lineEditChat->text().toStdString();
        LocalPlayer::getInstance()->sendMessage(protocolHandler.getTchatProtocol(LocalPlayer::getInstance()->getName(), msg));
        ui->lineEditChat->clear();
    }else {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Partie");
        msgBox.setText("Erreur");
        msgBox.setInformativeText("Vous ne pouvez pas envoyer un message vide !");
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Icon::Warning);
        msgBox.exec();
    }

}

