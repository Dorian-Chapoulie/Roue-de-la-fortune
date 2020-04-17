#ifndef PROTOCOLHANDLER_H
#define PROTOCOLHANDLER_H
#include <unordered_map>
#include "eventmanager.h"

#ifdef _WIN32
#include <winsock2.h>
#elif defined (linux)
typedef int SOCKET;
#endif

class ProtocolHandler
{
public:

    enum PROTOCOL_NAME {
        PLAYER_CONNECT_OK,
        PLAYER_CONNECT_FAIL,
        PLAYER_INSCRIPTION_OK,
        PLAYER_INSCRIPTION_FAIL,
        ASK_PSEUDO,
        NOTIFY_NEW_PLAYER,
        TCHAT,
        PLAYER_DISCONNECTED,
    };

    ProtocolHandler(EventManager* eventManager);    

    void callEventFromProtocol(std::string msg, SOCKET* socket);
    std::string getProcotol(PROTOCOL_NAME name) const;

private:
    std::unordered_map<PROTOCOL_NAME, std::string> protocoles;
    EventManager* eventManager;
};

#endif // PROTOCOLHANDLER_H
