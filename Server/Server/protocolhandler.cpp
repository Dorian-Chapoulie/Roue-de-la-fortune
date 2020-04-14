#include "protocolhandler.h"
#include "eventmanager.h"
#include <fstream>
#include <string>

ProtocolHandler::ProtocolHandler()
{
    protocoles.insert_or_assign(PROTOCOL_NAME::CREATED_GAME, "CG-");   
    protocoles.insert_or_assign(PROTOCOL_NAME::PLAYER_CONNECT_OK, "C-1");
    protocoles.insert_or_assign(PROTOCOL_NAME::PLAYER_CONNECT_FAIL, "C-0");
    protocoles.insert_or_assign(PROTOCOL_NAME::PLAYER_INSCRIPTION_OK, "I-1");
    protocoles.insert_or_assign(PROTOCOL_NAME::PLAYER_INSCRIPTION_FAIL, "I-0");
}
#include <iostream>
void ProtocolHandler::callEventFromProtocol(std::string msg, SOCKET* socket)
{
    EventManager* eventManager = EventManager::getInstance();

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


    } else if(msg.at(0) == 'I') {

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
    }
    else if (msg.at(0) == 'G') {
        eventManager->triggerEvent(EventManager::EVENT::GET_ALL_GAMES, socket);
    }
}

std::string ProtocolHandler::getProcotol(ProtocolHandler::PROTOCOL_NAME name) const
{
    return this->protocoles.at(name);
}
