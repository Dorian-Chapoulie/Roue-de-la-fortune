#include "game.h"
#include "ui_game.h"
#include "ui/servernavigator.h"
#include "event/eventmanager.h"
#include "protocol/protocolhandler.h"
#include "entity/localplayer.h"
#include "graphics/wheelfactory.h"
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
    connect(this, SIGNAL(notifySpinWheel(int)), this, SLOT(drawWheelScene(int)));
    connect(this, SIGNAL(notifyCleanScene()), this, SLOT(clearScene()));
    connect(this, SIGNAL(notifySetEnableWheel(bool)), this, SLOT(setEnableWheel(bool)));
    connect(this, SIGNAL(notifyMoneyChanged()), this, SLOT(displayMoney()));
    connect(this, SIGNAL(notifyWheelButtonAnimation(bool)), this, SLOT(changeWheelButtonColor(bool)));
    connect(this, SIGNAL(notifyRemoveLetter(char)), this, SLOT(removeLetter(char)));
    connect(this, SIGNAL(notifyUpdateBank()), this, SLOT(updateBank()));
    connect(this, SIGNAL(notifyMsgBox(QString)), this, SLOT(showMsgBox(QString)));
    connect(this, SIGNAL(notifySetComboBox()), this, SLOT(setComboBox()));

    ui->lineEditChat->setValidator(new QRegExpValidator(QRegExp("[A-Za-z0-9_ ]{0,50}"), this));    

    emit notifySetComboBox();

    ui->lineEditWord->setEnabled(false);
    ui->pushButtonVoyelle->setEnabled(false);
    ui->pushButtonConsonne->setEnabled(false);
    ui->pushButton->setEnabled(false);
    ui->buttonSpinWheel->setEnabled(false);


    setEvents();
    LocalPlayer::getInstance()->login();    


    scene = new QGraphicsScene(this);
    wheelScene = new QGraphicsScene(this);

    scene->setSceneRect(0, 0, 840, 430);
    wheelScene->setSceneRect(0, 0, 350, 270);
    this->ui->graphicsView->setScene(scene);
    this->ui->graphicsViewRoue->setScene(wheelScene);

    this->wheel = WheelFactory::getInstance()->getWheel(WheelFactory::WHEEL_ONE);
    wheel->setPosition(350 / 2, 270 /2 + 15);
    wheelScene->addItem(wheel->getItem());


    QPen pen;
    QBrush brush(Qt::red);
    brush.setStyle(Qt::SolidPattern);
    wheelScene->addRect(350/2 - 5, 0, 10, 10, pen, brush);

    wheelScene->addLine(350/2, 10, 350/2, 30);
    wheelScene->addLine(350/2, 30, 350/2 - 5, 20);
    wheelScene->addLine(350/2, 30, 350/2 + 5, 20);   

    drawScene();
}

Game::~Game()
{
    delete ui;
    delete wheel;
    delete scene;
    delete wheelScene;
    players.clear();
}

void Game::closeEvent(QCloseEvent *event)
{
    emit clearScene();
    delete scene;
    delete wheelScene;
    players.clear();
    cases.clear();
    delete wheel;

    LocalPlayer::getInstance()->disconnect();
    LocalPlayer::getInstance()->connectToBaseServer();

    ServerNavigator* serverNavigator = new ServerNavigator();
    serverNavigator->show();
    this->close();
}

void Game::setEvents() {

    EventManager::getInstance()->addListener(EventManager::EVENT::CONNEXION_SUCCESS, [&](void* idStr){
        int id = std::stoi(*static_cast<std::string*>(idStr));
        LocalPlayer::getInstance()->setId(id);
    });

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
        emit notifyCleanScene();
        while(!isSceneCleared) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        isSceneCleared = false;
        currentSentence = std::string(*reinterpret_cast<std::string*>(sentence));
        isQuickRiddle = true;
        prepareScene();
        emit notifyUpdateScene();
    });

    EventManager::getInstance()->addListener(EventManager::RECEIVE_SENTENCE_RIDDLE, [&](void* sentence){
        emit notifyCleanScene();
        while(!isSceneCleared) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        isSceneCleared = false;
        currentSentence = std::string(*reinterpret_cast<std::string*>(sentence));
        isQuickRiddle = false;
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
        bool canPlay = canPlayint > 0 ? true : false;
        emit notifyCanPlayValue(canPlay);
    });

    EventManager::getInstance()->addListener(EventManager::ENABLE_WHEEL, [&](void* isWheelEnabled){
        int wheelEnabled = std::stoi(*reinterpret_cast<std::string*>(isWheelEnabled));
        bool value = wheelEnabled > 0 ? true : false;        
        isWheelButtonClicked = false;

        std::thread threadAnimation([&](){
            bool value = true;
            while(!isWheelButtonClicked) {
                emit notifyWheelButtonAnimation(value);
                value = !value;
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            emit notifyWheelButtonAnimation(false);
            isWheelButtonClicked = false;
        });
        threadAnimation.detach();

        if(!value) {
            isWheelButtonClicked = true;
        }

        emit notifySetEnableWheel(value);
    });

    EventManager::getInstance()->addListener(EventManager::BAD_RESPONSE, [&](void*){
        emit notifyBadResponse();
    });

    EventManager::getInstance()->addListener(EventManager::DISPLAY_RESPONSE, [&](void*){
        for(Case* c : cases) {
            c->displayLetter();
        }
        emit notifyUpdateScene();
    });

    EventManager::getInstance()->addListener(EventManager::RECEIVE_LETTER, [&](void* data){
        std::string s_data = *reinterpret_cast<std::string*>(data);
        char c = s_data.at(0);
        s_data = s_data.substr(s_data.find("-") + 1);
        int position = std::stoi(s_data);

        if(!isQuickRiddle) {
            emit notifyRemoveLetter(c);
        }

        if(position != -1) {
            if(isQuickRiddle) {
                for(Case* c : cases) {
                    if(c->getId() == position){
                        c->displayLetter();
                        break;
                    }
                }
                emit notifyUpdateScene();
            }else {
                for(Case* c : cases) {
                    if(c->getId() == position){
                        c->displayLetterAnimation();
                        break;
                    }
                }

                emit notifyUpdateScene();
                std::this_thread::sleep_for(std::chrono::milliseconds(200));

                for(Case* c : cases) {
                    if(c->getId() == position){
                        c->displayLetter();
                        break;
                    }
                }
                emit notifyUpdateScene();
            }
        }

    });

    EventManager::getInstance()->addListener(EventManager::SPIN_WHEEL, [&](void* value){
        rotationValueWheel = std::stoi(*static_cast<std::string*>(value));
        std::thread threadTemp([&](){

            for(int i = 0; i < 360; i++) {
                emit notifySpinWheel(1);
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            }

            while(wheel->getRotationStep() != rotationValueWheel) {
                emit notifySpinWheel(1);
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            }

            ProtocolHandler protocol;
            LocalPlayer::getInstance()->sendMessage(protocol.getWheelSpinnedProtocol(wheel->getCaseFromRotation()));
        });
        threadTemp.detach();
    });

    EventManager::getInstance()->addListener(EventManager::EVENT::PLAYER_MONEY, [&](void* data){
        std::string stringData = *reinterpret_cast<std::string*>(data); //player-ammount
        int id = std::stoi(stringData.substr(0, stringData.find("-")));
        int ammount = std::stoi(stringData.substr(stringData.find("-") + 1, stringData.length() - stringData.find("-")));

        auto it = std::find_if(players.begin(), players.end(), [&](Player* p) {
            return id == p->getId();
        });
        if(it != players.end()) {
            Player *p = reinterpret_cast<Player*>(*it);
            p->setMoney(ammount);
            emit notifyMoneyChanged();
        }

    });

    EventManager::getInstance()->addListener(EventManager::EVENT::NEW_ROUND, [&](void* data){
        std::string roundNumber = *reinterpret_cast<std::string*>(data);
        emit notifyNewMessage(QString::fromStdString("[INFO]-Nouvelle manche:" + roundNumber));

        for(Player* p : players) {
            p->updateBank();
        }
        emit notifyMoneyChanged();
        emit notifyUpdateBank();
        emit notifySetComboBox();
    });

    EventManager::getInstance()->addListener(EventManager::EVENT::VICTORY, [&](void* data){
        QString str = QString::fromStdString(*reinterpret_cast<std::string*>(data));
        LocalPlayer::getInstance()->updateBank();
        str += QString::number(LocalPlayer::getInstance()->getBank());
        emit notifyMsgBox(str);
    });

    EventManager::getInstance()->addListener(EventManager::EVENT::LOOSE, [&](void* data){
        QString str = QString::fromStdString(*reinterpret_cast<std::string*>(data));
        LocalPlayer::getInstance()->updateBank();
        str += QString::number(LocalPlayer::getInstance()->getBank());
        emit notifyMsgBox(str);
    });
}

void Game::setComboBox()
{
    const char consonnes[19] = {'b', 'c', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 'm', 'n', 'p', 'q', 'r', 's', 't', 'v', 'x', 'z'};
    const char voyelles[6] = {'a', 'e', 'i','o', 'u', 'y'};

    ui->comboBoxVoyelle->clear();
    ui->comboBoxConsonne->clear();

    for(const auto& c : consonnes) {
        ui->comboBoxConsonne->addItem(QChar(c));
    }

    for(const auto& c : voyelles) {
        ui->comboBoxVoyelle->addItem(QChar(c));
    }

}

void Game::prepareScene() {

    mutex.lock();

    int sentenceLenght = currentSentence.length();
    int reste = 50 - sentenceLenght;

    int index = 0;
    int charPos = 0;
    for(int y = 0; y < 5; y++) {
        for(int x = 0; x < 10; x++) {
            if(charPos >= reste / 2 && index < sentenceLenght) {
                if(currentSentence.at(index) != ' ') {
                    cases.push_back(new Case(x * Case::width + (x * 4), y * Case::height + (y * 6), index, true));
                    cases.back()->setLetter(currentSentence.at(index));
                } else {
                    cases.push_back(new Case(x * Case::width + (x * 4) ,
                                     y * Case::height + (y * 6), index, false));
                    cases.back()->setLetter(currentSentence.at(index));
                }
                index++;
            }else {
                cases.push_back(new Case(x * Case::width + (x * 4) ,
                                 y * Case::height + (y * 6), -1, false));
            }
            charPos++;
        }
    }
    mutex.unlock();
}

void Game::drawScene()
{    
    mutex.lock();
    for(Case* c : cases) {
        c->drawBox(this->scene);
    }   
    mutex.unlock();
}

void Game::drawWheelScene(int value) {
    wheel->rotate(value);
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

    if(s == LocalPlayer::getInstance()->getId()) {
        players.push_back(LocalPlayer::getInstance());
    }else {
        players.push_back(new Player(name, s));
    }

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
                                + "<font color=\"Orange\">" + toDisplay);
    }else if(pseudo == "[INFO]") {
        ui->textBrowserChat->append("<font color=\"Green\"><b>" + QString::fromStdString(pseudo) + ":</b> "
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


    if(it == players.end()) {
        return;
    }

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
    }else {
        LocalPlayer::getInstance()->sendMessage(protocolHanlder.getSentenceRiddlePropositon(proposition));
    }
    ui->lineEditWord->clear();

    setCanPlay(false);
}

void Game::setCanPlay(bool value) {

    if(isQuickRiddle) {
        ui->pushButtonVoyelle->setEnabled(false);
        ui->pushButtonConsonne->setEnabled(false);
        ui->lineEditWord->setEnabled(value);
        ui->pushButton->setEnabled(value);
    }else {
        if(LocalPlayer::getInstance()->getMoney() >= 200) {
            ui->pushButtonVoyelle->setEnabled(value);
        }else {
            ui->pushButtonVoyelle->setEnabled(false);
        }
        ui->pushButtonConsonne->setEnabled(value);
        ui->lineEditWord->setEnabled(value);
        ui->pushButton->setEnabled(value);
    }

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

    if(isQuickRiddle)
        setCanPlay(true);
}

void Game::clearScene() {
    scene->clear();
    cases.clear();
    isSceneCleared = true;
}

void Game::on_buttonSpinWheel_clicked()
{
    ProtocolHandler protocol;
    LocalPlayer::getInstance()->sendMessage(protocol.getSpinWheelProtocol());
    ui->buttonSpinWheel->setEnabled(false);
    isWheelButtonClicked = true;
}

void Game::setEnableWheel(bool value)
{
    ui->buttonSpinWheel->setEnabled(value);
}

void Game::displayMoney()
{
    for(Player* p : players) {
        if(p->getName() == ui->labelPlayer1->text().toStdString()) {
            ui->labelMoneyP1->setText(QString::number(p->getMoney()));
        }else if(p->getName() == ui->labelPlayer2->text().toStdString()) {
            ui->labelMoneyP2->setText(QString::number(p->getMoney()));
        }else {
            ui->labelMoneyP3->setText(QString::number(p->getMoney()));
        }
    }
}

void Game::updateBank()
{
    for(Player* p : players) {
        if(p->getName() == ui->labelPlayer1->text().toStdString()) {
            ui->labelbankP1->setText(QString::number(p->getBank()));
        }else if(p->getName() == ui->labelPlayer2->text().toStdString()) {
            ui->labelbankP2->setText(QString::number(p->getBank()));
        }else {
            ui->labelbankP3->setText(QString::number(p->getBank()));
        }
    }
}

void Game::changeWheelButtonColor(bool value)
{
    if(value) {
        ui->buttonSpinWheel->setStyleSheet("QPushButton{ background-color: green }");
    }else {
        ui->buttonSpinWheel->setStyleSheet("QPushButton{ background-color: none }");
    }
}

void Game::removeLetter(char c)
{
    bool isVoyelle = false;    
    for(int i = 0; i < ui->comboBoxVoyelle->count(); i++) {
        std::string lettre = ui->comboBoxVoyelle->itemText(i).toStdString();
        if(lettre.at(0) == c) {
            ui->comboBoxVoyelle->removeItem(i);
            isVoyelle = true;
            break;
        }
    }

    if(!isVoyelle) {
        for(int i = 0; i < ui->comboBoxConsonne->count(); i++) {
            std::string lettre = ui->comboBoxConsonne->itemText(i).toStdString();
            if(lettre.at(0) == c) {
                ui->comboBoxConsonne->removeItem(i);
                break;
            }
        }
    }
}

void Game::showMsgBox(QString msg)
{
    QMessageBox msgBox;
    msgBox.setWindowTitle("Game");
    msgBox.setText("Fin du jeu");
    msgBox.setInformativeText(msg);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.setIcon(QMessageBox::Information);
    msgBox.exec();

    LocalPlayer::getInstance()->disconnect();
    LocalPlayer::getInstance()->connectToBaseServer();

    this->close();
}

void Game::on_pushButtonVoyelle_clicked()
{
    LocalPlayer::getInstance()->buyVoyelle();
    emit notifyMoneyChanged();
    emit setCanPlay(false);

    char letter = ui->comboBoxVoyelle->currentText().toStdString().at(0);

    ProtocolHandler protocol;
    LocalPlayer::getInstance()->sendMessage(protocol.getSendLetterProtocol(letter));
}

void Game::on_pushButtonConsonne_clicked()
{
    emit setCanPlay(false);

    char letter = ui->comboBoxConsonne->currentText().toStdString().at(0);

    ProtocolHandler protocol;
    LocalPlayer::getInstance()->sendMessage(protocol.getSendLetterProtocol(letter));
}
