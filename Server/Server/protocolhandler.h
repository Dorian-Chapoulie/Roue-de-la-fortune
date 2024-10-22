#ifndef PROTOCOLHANDLER_H
#define PROTOCOLHANDLER_H
#include <map>
#include "eventmanager.h"
#include "Player.h"

#ifdef _WIN32
#include <winsock2.h>
#elif defined (linux)
typedef int SOCKET;
#endif

class ProtocolHandler
{
public:
    //Every protocol that we will send to the client
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
        SPIN_WHEEL,
        SEND_SENTENCE_RIDDLE,
        ACTIVE_WHEEL,
        SEND_MONEY,
        NEW_ROUND,
        VICTORY,
    	LOOSE,
    };

    ProtocolHandler(EventManager* eventManager);    

    void callEventFromProtocol(std::string msg, SOCKET* socket);
    std::string getProcotol(PROTOCOL_NAME name) const;
    std::string getConnectionOKProtocol(std::string id);
    std::string getAskPseudoProtocol();
    std::string getNewPlayerProtocol(std::string pseudo, std::string id);
    std::string getPlayerDisconnectedProtocol(std::string id);
    std::string getVictoryProtocol();
    std::string getLooseProtocol();
	std::string getBadResponseProtocol();
    std::string getDisplayResponseProtocol();
    std::string getQuickRiddleProtocol(std::string& sentence);
    std::string getSendLetterProtocol(char& letter, int position);
    std::string getWinnerIdProtocol(int id);
    std::string getCanPlayProtocol(bool canPlay);
    std::string getServerChatProtocol(std::string msg);
    std::string getSpinWheelProtocol(int value);
    std::string getSentenceRiddleProtocol(std::string sentence);
    std::string getActivateWheelProtocol(bool isEnabled);
    std::string getSendMoneyProtocol(Player* p);
    std::string getNewRoundProtocol(int roundNumber);

private:
    std::map<PROTOCOL_NAME, std::string> protocoles;
    EventManager* eventManager;
};

#endif // PROTOCOLHANDLER_H
