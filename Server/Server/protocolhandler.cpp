#include "protocolhandler.h"
#include "eventmanager.h"
#include <fstream>
#include <string>

ProtocolHandler::ProtocolHandler(EventManager* eventManager)
{ 
    protocoles.insert_or_assign(PROTOCOL_NAME::PLAYER_CONNECT_OK, "C-1-");
    protocoles.insert_or_assign(PROTOCOL_NAME::PLAYER_CONNECT_FAIL, "C-0");
    protocoles.insert_or_assign(PROTOCOL_NAME::PLAYER_INSCRIPTION_OK, "I-1");
    protocoles.insert_or_assign(PROTOCOL_NAME::PLAYER_INSCRIPTION_FAIL, "I-0");
    protocoles.insert_or_assign(PROTOCOL_NAME::ASK_PSEUDO, "N");
    protocoles.insert_or_assign(PROTOCOL_NAME::NOTIFY_NEW_PLAYER, "NJ");
    protocoles.insert_or_assign(PROTOCOL_NAME::PLAYER_DISCONNECTED, "D-");
    protocoles.insert_or_assign(PROTOCOL_NAME::SEND_QUICK_RIDDLE, "Q-");
    protocoles.insert_or_assign(PROTOCOL_NAME::SEND_WORD, "W-");
    protocoles.insert_or_assign(PROTOCOL_NAME::SEND_WINNER_ID, "J-");
    this->eventManager = eventManager;
}

#include <iostream>
void ProtocolHandler::callEventFromProtocol(std::string msg, SOCKET* socket)
{

    if (msg.at(0) == 'C') {
        std::string line;
        std::ifstream myfile("players.txt");

        std::string username = msg.substr(2, msg.find_last_of('-') - 2);
        std::string password = msg.substr(msg.find_last_of('-') + 1, msg.length());
        std::string lineToFind = username + "\t" + password;
        bool isCredentialValid = false;

        if (myfile.is_open())
        {
            while (getline(myfile, line))
            {
                if (line == lineToFind) {
                    isCredentialValid = true;
                    break;
                }
            }
            myfile.close();
        }

        if (isCredentialValid) {
            eventManager->triggerEvent(EventManager::EVENT::PLAYER_CONNECT_OK, socket);
        }
        else {
            eventManager->triggerEvent(EventManager::EVENT::PLAYER_CONNECT_FAIL, socket);
        }


    }
    else if(msg.at(0) == 'I') {

        std::string line;
        std::ifstream myfile("players.txt");

        std::string username = msg.substr(2, msg.find_last_of('-') - 2);
        std::string password = msg.substr(msg.find_last_of('-') + 1, msg.length());        
        std::string lineToAppend = username + "\t" + password;        
        bool isCredentialValid = true;
        
        if (myfile.is_open())
        {

            while (getline(myfile, line))
            {
                std::string fileUsername = line.substr(0, line.find("\t"));
                if (fileUsername == username) {
                    isCredentialValid = false;
                }
            }
            myfile.close();

            std::ofstream myfile;
            myfile.open("players.txt", std::ofstream::out | std::ofstream::app);

            if (isCredentialValid) {    
                myfile << lineToAppend + "\n";
            }

            myfile.close();
        }

        if (isCredentialValid) {
            eventManager->triggerEvent(EventManager::EVENT::PLAYER_INSCRIPTION_OK, socket);
        }
        else {
            eventManager->triggerEvent(EventManager::EVENT::PLAYER_INSCRIPTION_FAIL, socket);
        }

    }
    else if(msg.at(0) == 'P') {       
        std::string gameName = msg.substr(msg.find("P-") + 2, msg.length());
        eventManager->triggerEvent(EventManager::EVENT::CREATE_GAME, &gameName);          
        eventManager->triggerEvent(EventManager::EVENT::GET_ALL_GAMES, socket);
    }
    else if(msg.at(0) == 'G') {
        eventManager->triggerEvent(EventManager::EVENT::GET_ALL_GAMES, socket);
    }
    else if(msg.at(0) == 'N') {
        std::string pseudo = msg.substr(2);
        pseudo += "-" + std::to_string(*reinterpret_cast<SOCKET*>(socket));
        eventManager->triggerEvent(EventManager::EVENT::ASK_PSEUDO, &pseudo);
    }
    else if(msg.at(0) == 'T') {
        eventManager->triggerEvent(EventManager::EVENT::TCHAT, &std::string(msg));
    }
    else if(msg.at(0) == 'D') {
        SOCKET id = static_cast<SOCKET>(std::stoi(msg.substr(2)));
        eventManager->triggerEvent(eventManager->PLAYER_DISCONNECTED, &id);
    }
    else if (msg.at(0) == 'Q') {
        std::string data = msg.substr(2);
        data += "-" + std::to_string(*socket);
        eventManager->triggerEvent(eventManager->PLAYER_QUICK_RIDDLE, &data);
    }
}

std::string ProtocolHandler::getProcotol(ProtocolHandler::PROTOCOL_NAME name) const
{
    return this->protocoles.at(name);
}

std::string ProtocolHandler::getQuickRiddleProtocol(std::string& sentence)
{
    return this->protocoles.at(PROTOCOL_NAME::SEND_QUICK_RIDDLE) + sentence;
}

std::string ProtocolHandler::getSendLetterProtocol(char& letter, int position)
{
    return this->protocoles.at(PROTOCOL_NAME::SEND_WORD) + letter + "-" + std::to_string(position);
}

std::string ProtocolHandler::getWinnerIdProtocol(int id)
{
    return this->protocoles.at(PROTOCOL_NAME::SEND_WINNER_ID) + std::to_string(id);
}
