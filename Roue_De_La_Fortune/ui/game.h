#ifndef GAME_H
#define GAME_H

#include <QGraphicsScene>
#include <QWidget>
#include <mutex>
#include "graphics/case.h"
#include "entity/player.h"
#include "graphics/wheel.h"

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
    void drawWheelScene(int);
    void clearScene();
    void setEnableWheel(bool);

    void on_pushButtonChat_clicked();
    void on_pushButton_clicked();
    void on_buttonSpinWheel_clicked();

signals:
    void notifyNewPlayer(QString);
    void notifyNewMessage(QString);
    void notifyPlayerDisconnected(int);
    void notifyUpdateScene();
    void notifyCanPlayValue(bool);
    void notifyWinner(int);
    void notifyBadResponse();
    void notifySpinWheel(int);
    void notifyCleanScene();
    void notifySetEnableWheel(bool);

private:
    Ui::Game *ui;
    QGraphicsScene* scene;
    QGraphicsScene* wheelScene;

    std::string currentSentence;

    std::vector<Player*> players;
    std::vector<Case*> cases;
    std::mutex mutex;

    bool isQuickRiddle = true;
    bool isSceneCleared = false;
    int rotationValueWheel = 0;

    Wheel* wheel = nullptr;

    std::vector<char> getLettersFromString(std::string s);
    void prepareScene();
    void setEvents();
};

#endif // GAME_H
