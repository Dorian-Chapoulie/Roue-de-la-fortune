#ifndef GAME_H
#define GAME_H

#include <QGraphicsScene>
#include <QWidget>
#include "entity/player.h"

namespace Ui {
class Game;
}

class Game : public QWidget
{
    Q_OBJECT

public:
    explicit Game(QWidget *parent = nullptr);
    ~Game();

private slots:
    void addNewPlayer(QString);
    void addMessageToChat(QString);
    void removePlayer(int);

    void on_pushButtonChat_clicked();

signals:
    void notifyNewPlayer(QString);
    void notifyNewMessage(QString);
    void notifyPlayerDisconnected(int);

private:
    Ui::Game *ui;
    QGraphicsScene* scene;
    std::vector<Player*> players;


    void drawScene();
};

#endif // GAME_H
