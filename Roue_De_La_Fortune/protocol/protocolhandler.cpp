#include "protocol/protocolhandler.h"
#include "event/eventmanager.h"
#include <string>

ProtocolHandler::ProtocolHandler()
{
    protocoles.insert_or_assign(PROTOCOL_NAME::GET_ALL_GAMES, "G");
}

void ProtocolHandler::callEventFromProtocol(std::string msg)
{
    EventManager* eventManager = EventManager::getInstance();

    if(msg.at(0) == 'C') {//Connection
        if(msg == "C-0") {
            eventManager->triggerEvent(eventManager->CONNEXION_FAILURE, "Identifiant ou mot de passe incorrect.");
        }else if (msg.find("C-1-") != std::string::npos){
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
    }else if(msg.at(0) == 'N') {
        if(msg.length() >= 2 && msg.at(1) == 'J') {
            std::string name = msg.substr(3);
            eventManager->triggerEvent(eventManager->NEW_PLAYER, name);
        }else {
            eventManager->triggerEvent(eventManager->ASK_PSEUDO);
        }
    }else if(msg.at(0) == 'T') {
        std::string data = msg.substr(2);
        eventManager->triggerEvent(eventManager->TCHAT, data);
    }else if(msg.at(0) == 'D') {
        std::string id = msg.substr(2);
        eventManager->triggerEvent(eventManager->PLAYER_DISCONNECT, id);
    }else if(msg.at(0) == 'Q'){ // quick riddle
        std::string phrase = msg.substr(2);
        eventManager->triggerEvent(eventManager->RECEIVE_QUICK_RIDDLE, phrase);
    }else if(msg.at(0) == 'W') { // receive letter and pos
        std::string data = msg.substr(2);
        eventManager->triggerEvent(eventManager->RECEIVE_LETTER, data);
    }else if(msg.at(0) == 'J') { //gagnant de l'énigme actuelle
        eventManager->triggerEvent(eventManager->WINNER, msg.substr(2));
    }else if(msg.at(0) == 'B') { //canPLay
        eventManager->triggerEvent(eventManager->CAN_PLAY, msg.substr(2));
    }else if(msg.at(0) == 'M') { //mauvaise réponse
        eventManager->triggerEvent(eventManager->BAD_RESPONSE);
    }else if(msg.at(0) == 'F') { //afficher la réponse
        eventManager->triggerEvent(eventManager->DISPLAY_RESPONSE);
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


std::string ProtocolHandler::getPseudoProtocol(std::string &pseudo)
{
    protocoles.insert_or_assign(PROTOCOL_NAME::ASK_PSEUDO, "N-" + pseudo);
    return protocoles[PROTOCOL_NAME::ASK_PSEUDO];
}

std::string ProtocolHandler::getTchatProtocol(std::string& pseudo, std::string &message)
{
    protocoles.insert_or_assign(PROTOCOL_NAME::TCHAT, "T-" + pseudo + "-" + message);
    return protocoles[PROTOCOL_NAME::TCHAT];
}

std::string ProtocolHandler::getAllGamesProtocol() const
{
    return protocoles.at(PROTOCOL_NAME::GET_ALL_GAMES);
}

#include <iostream>
std::string ProtocolHandler::getQuickRiddlePropositon(std::string &sentence)
{
    protocoles.insert_or_assign(PROTOCOL_NAME::SEND_QUICK_RIDDLE_PROPOSITION, "Q-" + sentence);
    return protocoles.at(PROTOCOL_NAME::SEND_QUICK_RIDDLE_PROPOSITION);
}
