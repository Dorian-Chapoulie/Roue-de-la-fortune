#ifndef PROTOCOLHANDLER_H
#define PROTOCOLHANDLER_H
#include <map>

class ProtocolHandler
{
public:

    enum PROTOCOL_NAME {
        CONNEXION,
        INSCRIPTION,
        CREATE_GAME,
        ASK_PSEUDO,
        TCHAT,
        GET_ALL_GAMES,
        SEND_QUICK_RIDDLE_PROPOSITION,
        SEND_SENTENCE_RIDDLE_PROPOSITION,
        SPIN_WHEEL,
        WHEEL_SPINNED,
        SEND_LETTER,
    };

    ProtocolHandler();

    std::string getLoginProtocol(std::string& pseudo, std::string& password);
    std::string getInscriptionProtocol(std::string& pseudo, std::string& password);
    std::string getCreateGameProtocol(std::string& gameName);
    std::string getPseudoProtocol(std::string& pseudo);
    std::string getTchatProtocol(std::string& pseudo, std::string& message);
    std::string getAllGamesProtocol() const;
    std::string getQuickRiddlePropositon(std::string& sentence);
    std::string getSentenceRiddlePropositon(std::string& sentence);
    std::string getSpinWheelProtocol();
    std::string getWheelSpinnedProtocol(std::string value);
    std::string getSendLetterProtocol(char c);

    void callEventFromProtocol(std::string msg);

private:
    std::map<PROTOCOL_NAME, std::string> protocoles;
};

#endif // PROTOCOLHANDLER_H
