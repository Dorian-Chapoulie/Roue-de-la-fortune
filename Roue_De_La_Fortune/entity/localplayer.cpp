#include "entity/localplayer.h"
#include "protocol/protocolhandler.h"
#include "config/config.h"

LocalPlayer* LocalPlayer::instance = nullptr;
std::string LocalPlayer::tempName;
int LocalPlayer::id = -1;

void LocalPlayer::setTempName(std::string name) {
    LocalPlayer::tempName = name;
    if(instance == nullptr) {
        instance = getInstance();
    }
    instance->setName(name);
}

void LocalPlayer::setId(int id)
{
    LocalPlayer::id = id;
    instance->id = id;
}

LocalPlayer* LocalPlayer::getInstance() {
    if(LocalPlayer::instance == nullptr)
        LocalPlayer::instance = new LocalPlayer(LocalPlayer::tempName, LocalPlayer::id);
    return LocalPlayer::instance;
}

LocalPlayer::LocalPlayer(std::string &name, int id)
    : Player(name, id), TCPClient(Config::getInstance()->baseServerIP, Config::getInstance()->baseServerPort)
{
}

void LocalPlayer::login()
{
    ProtocolHandler protocol;
    sendMessage(protocol.getLoginProtocol(this->getName(), password));
}

void LocalPlayer::setPassword(std::string&& password)
{
    this->password = password;
}

std::string LocalPlayer::getPassword() const
{
    return this->password;
}

LocalPlayer::~LocalPlayer() {

}
