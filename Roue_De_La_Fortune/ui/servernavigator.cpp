#include "servernavigator.h"
#include "ui_servernavigator.h"
#include "entity/localplayer.h"
#include <QTableWidgetItem>
#include "event/eventmanager.h"
#include "protocol/protocolhandler.h"
#include "ui/game.h"

#include <QMessageBox>
#include <iostream>

ServerNavigator::ServerNavigator(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ServerNavigator)
{
    ui->setupUi(this);

    connect(this, SIGNAL(updateList(QString)), this, SLOT(addServerInList(QString)));

    EventManager::getInstance()->addListener(EventManager::EVENT::GAMES_LIST, [&](void* msg){
        emit updateList(QString::fromStdString(*reinterpret_cast<std::string*>(msg)));
    });

    headerList.append("Nom");
    headerList.append("Joueurs");
    headerList.append("Spectateurs");
    headerList.append("IP");
    headerList.append("Port");

    ui->tableWidget->setColumnCount(headerList.size());
    ui->tableWidget->setHorizontalHeaderLabels(this->headerList);
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);

    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);

}

ServerNavigator::~ServerNavigator()
{
    delete ui;
}

void ServerNavigator::showEvent(QShowEvent *)
{
    std::thread threadGetServerGames([&](){
        while(this->isVisible()) {
            LocalPlayer::getInstance()->sendMessage("G"); //TODO: No duplicates
            std::this_thread::sleep_for (std::chrono::seconds(1));
        }
    });
    threadGetServerGames.detach();
}

void ServerNavigator::addServerInList(QString msg)
{

    int pos[5];
    int index = 0;
    for(int i = 0; i < msg.length(); i++) {
        if(msg.at(i) == '-') {
            pos[index++] = i;
        }
    }

    std::string temp = msg.toStdString();
    std::string name = temp.substr(pos[0] + 1, pos[1] - 2);
    std::string nbPlayer = temp.substr(pos[1] + 1, pos[2] - pos[1] - 1);
    std::string nbSpec = temp.substr(pos[2] + 1, pos[3] - pos[2] - 1);
    std::string ip = temp.substr(pos[3] + 1, pos[4] - pos[3] - 1);
    std::string port = temp.substr(pos[4] + 1);
    std::vector<std::string> list = {name, nbPlayer, nbSpec, ip, port};


    bool addGame = true;
    for (int i = 0; i < ui->tableWidget->rowCount() ; ++i) {
        std::string tmpIP = ui->tableWidget->model()->index(i, 3).data().toString().toStdString();
        int tmpPort = ui->tableWidget->model()->index(i, 4).data().toInt();
        if(tmpIP == ip && tmpPort == std::stoi(port)) {
            addGame = false;
        }
    }

    if(addGame) {
        ui->tableWidget->setColumnWidth(0, name.length() * 9);

        ui->tableWidget->insertRow(ui->tableWidget->rowCount());
        for(int i = 0; i < headerList.size(); i++) {
            QTableWidgetItem item(QString::fromStdString(list.at(i)));
            item.setFlags(item.flags() & ~(Qt::ItemIsEditable));
            ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, i, new QTableWidgetItem(item));
        }
    }

}

void ServerNavigator::on_rejoindreButton_clicked()
{    
    if(ui->tableWidget->selectionModel()->selectedRows().size() > 0) {

        int row = ui->tableWidget->selectionModel()->selectedRows().at(0).row();

        std::string ip =  ui->tableWidget->model()->index(row, 3).data().toString().toStdString();
        int port =  ui->tableWidget->model()->index(row, 4).data().toInt();

        if(LocalPlayer::getInstance()->connectToServer(ip, port)) {

           Game* game = new Game();
           game->show();
           this->close();

        }else {
            QMessageBox msgBox;
            msgBox.setWindowTitle("Navigateur de serveurs");
            msgBox.setText("Erreur");
            msgBox.setInformativeText("Impossible de rejoindre la partie");
            msgBox.setDefaultButton(QMessageBox::Ok);
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.exec();
        }

    }else {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Navigateur de serveurs");
        msgBox.setText("Erreur");
        msgBox.setInformativeText("Veuillez sélectionner une partie");
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
    }


}

void ServerNavigator::on_createGameButton_clicked()
{
    ProtocolHandler protoclolHandler;
    std::string gameName = ui->lineEdit->text().toStdString();
    LocalPlayer::getInstance()->sendMessage(protoclolHandler.getCreateGameProtocol(gameName));
}
