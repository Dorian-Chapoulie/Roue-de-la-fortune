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
    connect(this, SIGNAL(notifyUpdateScene()), this, SLOT(drawScene()));
    connect(this, SIGNAL(notifyCanPlayValue(bool)), this, SLOT(setCanPlay(bool)));
    connect(this, SIGNAL(notifyWinner(int)), this, SLOT(diaplayWinner(int)));
    connect(this, SIGNAL(notifyBadResponse()), this, SLOT(diaplayBadResponse()));

    ui->lineEditChat->setValidator(new QRegExpValidator(QRegExp("[A-Za-z0-9_ ]{0,50}"), this));    

    const char consonnes[19] = {'b', 'c', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 'm', 'n', 'p', 'q', 'r', 's', 't', 'v', 'x', 'z'};
    const char voyelles[6] = {'a', 'e', 'i','o', 'u', 'y'};

    for(const auto& c : consonnes) {
        ui->comboBoxConsonne->addItem(QChar(c));
    }

    for(const auto& c : voyelles) {
        ui->comboBoxVoyelle->addItem(QChar(c));
    }

    ui->lineEditWord->setEnabled(false);
    ui->pushButtonVoyelle->setEnabled(false);
    ui->pushButtonConsonne->setEnabled(false);
    ui->pushButton->setEnabled(false);


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

    EventManager::getInstance()->addListener(EventManager::RECEIVE_QUICK_RIDDLE, [&](void* sentence){
        currentSentence = std::string(*reinterpret_cast<std::string*>(sentence));
        isQuickRiddle = true;
        prepareScene();
        emit notifyUpdateScene();
    });

    EventManager::getInstance()->addListener(EventManager::WINNER, [&](void* id){
        int t = std::stoi(*static_cast<std::string*>(id));
        auto it = std::find_if(players.begin(), players.end(), [&](Player* p) {
            return p->getId() == static_cast<SOCKET>(t);
        });
        if(it != players.end()) {
            emit notifyWinner(reinterpret_cast<Player*>(*it)->getId());
        }
    });

    EventManager::getInstance()->addListener(EventManager::CAN_PLAY, [&](void* canPlayValue){
        int canPlayint = std::stoi(*reinterpret_cast<std::string*>(canPlayValue));
        bool canPlay = canPlayint == 1 ? true : false;
        emit notifyCanPlayValue(canPlay);
    });

    EventManager::getInstance()->addListener(EventManager::BAD_RESPONSE, [&](void*){
        emit notifyBadResponse();
    });

    EventManager::getInstance()->addListener(EventManager::DISPLAY_RESPONSE, [&](void*){
        for(Case& c : cases) {
            c.displayLetter();
        }
        emit notifyUpdateScene();
    });

    EventManager::getInstance()->addListener(EventManager::RECEIVE_LETTER, [&](void* data){
        std::string s_data = *reinterpret_cast<std::string*>(data);
        char c = s_data.at(0);
        s_data = s_data.substr(s_data.find("-") + 1);
        int position = std::stoi(s_data);

        for(Case& c : cases) {
            if(c.getId() == position){
                c.displayLetter();
                break;
            }
        }
        emit notifyUpdateScene();

    });

    scene = new QGraphicsScene(this);
    scene->setSceneRect(0, 0, 840, 430);
    this->ui->graphicsView->setScene(scene);


    drawScene();

}

Game::~Game()
{
    delete ui;
}

void Game::prepareScene() {

    cases.clear();



    int sentenceLenght = currentSentence.length();
    int reste = 50 - sentenceLenght;

    int index = 0;
    int charPos = 0;
    for(int y = 0; y < 5; y++) {
        for(int x = 0; x < 10; x++) {
            if(charPos >= reste / 2 && index < sentenceLenght) {
                if(currentSentence.at(index) != ' ') {
                    cases.push_back(Case(x * Case::width + (x * 4), y * Case::height + (y * 6), index, true));
                    cases.back().setLetter(currentSentence.at(index));
                } else {
                    cases.push_back(Case(x * Case::width + (x * 4) ,
                                     y * Case::height + (y * 6), index, false));
                    cases.back().setLetter(currentSentence.at(index));
                }
                index++;
            }else {
                cases.push_back(Case(x * Case::width + (x * 4) ,
                                 y * Case::height + (y * 6), -1, false));
            }
            charPos++;
        }
    }
}

void Game::drawScene()
{    
    for(Case& c : cases) {
        c.drawBox(this->scene);
    }
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
    QString toDisplay = QString::fromStdString(message).toUtf8();

    if(pseudo == "[Serveur]") {
        ui->textBrowserChat->append("<font color=\"Red\"><b>" + QString::fromStdString(pseudo) + ":</b> "
                                + "<font color=\"Black\">" + toDisplay);
    }else {
        ui->textBrowserChat->append("<font color=\"Grey\"><b>" + QString::fromStdString(pseudo) + ":</b> "
                                + "<font color=\"Black\">" + toDisplay);
    }
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

std::vector<char> Game::getLettersFromString(std::string s)
{
    std::vector<char> ret;
    for(char c : s) {
        auto it = std::find(ret.begin(), ret.end(), c);
        if(it == ret.end()) {
            ret.push_back(c);
        }
    }

    return ret;
}

void Game::on_pushButton_clicked()
{
    ProtocolHandler protocolHanlder;
    std::string proposition = ui->lineEditWord->text().toLower().toStdString();
    if(isQuickRiddle) {        
        LocalPlayer::getInstance()->sendMessage(protocolHanlder.getQuickRiddlePropositon(proposition));
    }
    ui->lineEditWord->clear();

    setCanPlay(false);
}

void Game::setCanPlay(bool value) {
    ui->lineEditWord->setEnabled(value);
    ui->pushButtonVoyelle->setEnabled(value);
    ui->pushButtonConsonne->setEnabled(value);
    ui->pushButton->setEnabled(value);
}

void Game::diaplayWinner(int id) {

    auto it = std::find_if(players.begin(), players.end(), [&](Player* p) {
        return p->getId() == static_cast<SOCKET>(id);
    });

    QMessageBox msgBox;
    msgBox.setWindowTitle("Game");
    msgBox.setText("Winner");
    msgBox.setInformativeText(QString::fromStdString(reinterpret_cast<Player*>(*it)->getName()) + " à gagné la manche !");
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.setIcon(QMessageBox::Information);
    msgBox.exec();
}

void Game::diaplayBadResponse() {
    QMessageBox msgBox;
    msgBox.setWindowTitle("Game");
    msgBox.setText("Proposition");
    msgBox.setInformativeText("Ce n'est pas la bonne réponse !");
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.setIcon(QMessageBox::Information);
    msgBox.exec();

    setCanPlay(true);
}
