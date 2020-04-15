#ifndef GAME_H
#define GAME_H

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

signals:
    void notifyNewPlayer(QString);

private:
    Ui::Game *ui;

    std::vector<Player*> players;
};

#endif // GAME_H
