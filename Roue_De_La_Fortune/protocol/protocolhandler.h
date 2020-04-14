#ifndef PROTOCOLHANDLER_H
#define PROTOCOLHANDLER_H
#include <unordered_map>

class ProtocolHandler
{
public:

    enum PROTOCOL_NAME {
        CONNEXION,
        INSCRIPTION,
        CREATE_GAME,
    };

    ProtocolHandler();

    std::string getLoginProtocol(std::string& pseudo, std::string& password);
    std::string getInscriptionProtocol(std::string& pseudo, std::string& password);
    std::string getCreateGameProtocol(std::string& gameName);

    void callEventFromProtocol(std::string msg);

private:
    std::unordered_map<PROTOCOL_NAME, std::string> protocoles;
};

#endif // PROTOCOLHANDLER_H
