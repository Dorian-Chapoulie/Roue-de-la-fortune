#include "protocol/protocolhandler.h"
#include "event/eventmanager.h"

ProtocolHandler::ProtocolHandler()
{
}

void ProtocolHandler::callEventFromProtocol(std::string msg)
{
    EventManager* eventManager = EventManager::getInstance();

    if(msg.at(0) == 'C') {//Connection
        if(msg == "C-0") {
            eventManager->triggerEvent(eventManager->CONNEXION_FAILURE, "Identifiant ou mot de passe incorrect.");
        }else if (msg == "C-1"){
            eventManager->triggerEvent(eventManager->CONNEXION_SUCCESS);
        }
    }else if (msg.at(0) == 'I') {//Inscription
        if(msg == "I-1") {
            eventManager->triggerEvent(eventManager->INSCRIPTION_SUCCESS);
        }else {
            eventManager->triggerEvent(eventManager->INSCRIPTION_FAILURE, "Pseudo déjà pris.");
        }
    }else if(msg.at(0) == 'G') {
        eventManager->triggerEvent(eventManager->GAMES_LIST, msg);
    }
}

std::string ProtocolHandler::getLoginProtocol(std::string& pseudo, std::string& password) {
     protocoles.insert_or_assign(PROTOCOL_NAME::CONNEXION, "C-" + pseudo + "-" + password);
     return protocoles.at(PROTOCOL_NAME::CONNEXION);
}

std::string ProtocolHandler::getInscriptionProtocol(std::string &pseudo, std::string &password)
{
    protocoles.insert_or_assign(PROTOCOL_NAME::INSCRIPTION, "I-" + pseudo + "-" + password);
    return protocoles.at(PROTOCOL_NAME::INSCRIPTION);
}

std::string ProtocolHandler::getCreateGameProtocol(std::string &gameName)
{
    protocoles.insert_or_assign(PROTOCOL_NAME::CREATE_GAME, "P-" + gameName);
    return protocoles[PROTOCOL_NAME::CREATE_GAME];
}
