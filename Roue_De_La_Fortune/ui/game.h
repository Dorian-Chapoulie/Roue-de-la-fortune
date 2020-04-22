#ifndef GAME_H
#define GAME_H

#include <QGraphicsScene>
#include <QWidget>
#include "graphics/case.h"
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
    void drawScene();
    void setCanPlay(bool);
    void diaplayWinner(int);
    void diaplayBadResponse();

    void on_pushButtonChat_clicked();
    void on_pushButton_clicked();

signals:
    void notifyNewPlayer(QString);
    void notifyNewMessage(QString);
    void notifyPlayerDisconnected(int);
    void notifyUpdateScene();
    void notifyCanPlayValue(bool);
    void notifyWinner(int);
    void notifyBadResponse();

private:
    Ui::Game *ui;
    QGraphicsScene* scene;

    std::string currentSentence;

    std::vector<Player*> players;
    std::vector<Case> cases;

    bool isQuickRiddle = true;

    std::vector<char> getLettersFromString(std::string s);
    void prepareScene();
};

#endif // GAME_H
