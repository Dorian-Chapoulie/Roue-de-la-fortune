#include "entity/localplayer.h"
#include "protocol/protocolhandler.h"
#include "config/config.h"

LocalPlayer* LocalPlayer::instance = nullptr;
std::string LocalPlayer::name;

void LocalPlayer::setName(std::string &&name) {
    LocalPlayer::name = name;
}

LocalPlayer* LocalPlayer::getInstance() {
    if(LocalPlayer::instance == nullptr)
        LocalPlayer::instance = new LocalPlayer(LocalPlayer::name);
    return LocalPlayer::instance;
}

LocalPlayer::LocalPlayer(std::string &name)
    : Player(name), TCPClient(Config::getInstance()->baseServerIP, Config::getInstance()->baseServerPort)
{
}

void LocalPlayer::login()
{
    ProtocolHandler protocol;
    sendMessage(protocol.getLoginProtocol(name, password));
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
