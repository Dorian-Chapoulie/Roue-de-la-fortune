#ifndef PLAYER_H
#define PLAYER_H
#include <string>

#ifdef WIN32
    #include <winsock2.h>
#elif defined (linux)
    typedef int SOCKET;
#endif

class Player  {

public:
    Player(std::string& name, SOCKET id);
    Player(std::string name);

    std::string& getName();
    SOCKET getId();

    void setName(std::string name);
    void setMoney(int ammount);
    void buyVoyelle();
    void setId(int id);
    int getMoney();

private:
    std::string name;
    int money = 0;
    SOCKET id = -1;
};

#endif // PLAYER_H
