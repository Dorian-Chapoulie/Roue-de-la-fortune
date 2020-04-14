#include "servernavigator.h"
#include "ui_servernavigator.h"
#include "entity/localplayer.h"
#include <QTableWidgetItem>
#include "event/eventmanager.h"

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

    for(int i = 0; i < 10; i++) {
        Sleep(100);
        LocalPlayer::getInstance()->sendMessage("P-partie de dorian n" + std::to_string(i));
    }
    Sleep(1000);
    LocalPlayer::getInstance()->sendMessage("G");
}

ServerNavigator::~ServerNavigator()
{
    delete ui;
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
    ui->tableWidget->setColumnWidth(0, name.length() * 9);

    ui->tableWidget->insertRow(ui->tableWidget->rowCount());
    for(int i = 0; i < headerList.size(); i++) {
        QTableWidgetItem item(QString::fromStdString(list.at(i)));
        item.setFlags(item.flags() & ~(Qt::ItemIsEditable));
        ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, i, new QTableWidgetItem(item));
    }


}
#include <iostream>
void ServerNavigator::on_rejoindreButton_clicked()
{
    bool b = LocalPlayer::getInstance()->connectToServer("localhost", 25566);
    std::cout << b << std::endl;
}
