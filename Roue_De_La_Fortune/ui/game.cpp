#include "game.h"
#include "ui_game.h"
#include "event/eventmanager.h"
#include "protocol/protocolhandler.h"
#include "entity/localplayer.h"

#include <iostream>

Game::Game(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Game)
{
    ui->setupUi(this);

    connect(this, SIGNAL(notifyNewPlayer(QString)), this, SLOT(addNewPlayer(QString)));

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
        emit notifyNewPlayer(QString::fromStdString(newPlayerNameAndId));
    });

}

Game::~Game()
{
    delete ui;
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

}
