#ifndef LOCALPLAYER_H
#define LOCALPLAYER_H
#include "entity/player.h"
#include "network/tcpclient.h"

class LocalPlayer : public Player, public TCPClient {

public:
    static LocalPlayer* getInstance();
    static void setTempName(std::string name);
    static void setId(int id);

    void login();
    void setPassword(std::string&& password);
    std::string getPassword() const;

    ~LocalPlayer();

private:
    LocalPlayer(std::string& name, int id);

    bool isConnected = false;
    std::string password;

private:
    static LocalPlayer* instance;
    static std::string tempName;
    static int id;
};

#endif // LOCALPLAYER_H
