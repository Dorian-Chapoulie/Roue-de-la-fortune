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
        SEND_QUICK_RIDDLE,
        SEND_WORD,
        QUICK_RIDDLE_FOUND,
    	SEND_WINNER_ID,
    	BOOL_CAN_PLAY,
    	SERVER_CHAT,
    	BAD_RESPONSE,
    	DISPLAY_RESPONSE,
    };

    ProtocolHandler(EventManager* eventManager);    

    void callEventFromProtocol(std::string msg, SOCKET* socket);
    std::string getProcotol(PROTOCOL_NAME name) const;
    std::string getQuickRiddleProtocol(std::string& sentence);
    std::string getSendLetterProtocol(char& letter, int position);
    std::string getWinnerIdProtocol(int id);
    std::string getCanPlayProtocol(bool canPlay);
    std::string getServerChatProtocol(std::string msg);

private:
    std::unordered_map<PROTOCOL_NAME, std::string> protocoles;
    EventManager* eventManager;
};

#endif // PROTOCOLHANDLER_H
