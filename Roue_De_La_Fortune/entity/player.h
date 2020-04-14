#ifndef PLAYER_H
#define PLAYER_H
#include <string>

class Player  {

public:
    Player(std::string& name);

    std::string getName() const;

private:
    std::string name;
    int money = 0;

};

#endif // PLAYER_H
