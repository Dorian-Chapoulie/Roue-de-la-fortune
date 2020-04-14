#ifndef PROTOCOLHANDLER_H
#define PROTOCOLHANDLER_H
#include <unordered_map>

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
        CREATED_GAME,        
    };

    ProtocolHandler();    

    void callEventFromProtocol(std::string msg, SOCKET* socket);
    std::string getProcotol(PROTOCOL_NAME name) const;

private:
    std::unordered_map<PROTOCOL_NAME, std::string> protocoles;
};

#endif // PROTOCOLHANDLER_H
