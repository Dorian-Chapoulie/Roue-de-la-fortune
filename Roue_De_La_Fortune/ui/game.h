#ifndef GAME_H
#define GAME_H

#include <QGraphicsScene>
#include <QWidget>
#include <mutex>
#include "graphics/case.h"
#include "entity/player.h"
#include "graphics/wheel.h"
#include "sound/soundbank.h"

namespace Ui {
class Game;
}

class Game : public QWidget
{
    Q_OBJECT

public:
    explicit Game(QWidget *parent = nullptr);
    ~Game();

    void closeEvent(QCloseEvent *event) override;

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
    void clearWheelScene();
    void updateWheelScene();
    void setEnableWheel(bool);
    void displayMoney();
    void updateBank();
    void changeWheelButtonColor(bool);
    void removeLetter(char);  
    void showMsgBox(QString);
    void setComboBox();
    void playSound(SoundBank::SOUND sound);

    void on_pushButtonChat_clicked();
    void on_pushButton_clicked();
    void on_buttonSpinWheel_clicked();
    void on_pushButtonVoyelle_clicked();

    void on_pushButtonConsonne_clicked();

signals:
    void notifyNewPlayer(QString);
    void notifyNewMessage(QString);
    void notifyPlayerDisconnected(int);
    void notifyUpdateScene();
    void notifyUpdateWheelScene();
    void notifyCanPlayValue(bool);
    void notifyWinner(int);
    void notifyBadResponse();
    void notifySpinWheel(int);
    void notifyCleanScene();
    void notifyCleanWheelScene();
    void notifySetEnableWheel(bool);
    void notifyMoneyChanged();
    void notifyWheelButtonAnimation(bool);
    void notifyRemoveLetter(char);
    void notifyUpdateBank();
    void notifyMsgBox(QString);
    void notifySetComboBox();
    void notifyPlaySound(SoundBank::SOUND sound);

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
    bool isWheelSceneCleared = false;
    bool isWheelButtonClicked = false;
    bool isMyTurn = false;
    bool isWheelFinishedSpin = true;
    int rotationValueWheel = 0;

    Wheel* wheel = nullptr;

    std::vector<char> getLettersFromString(std::string s);
    void prepareScene();
    void setEvents();
};

#endif // GAME_H
