#include <iostream>
#include <vector>

#include "tcp_server.h"
#include "Game.h"
#include "eventmanager.h"
#include "protocolhandler.h"

#include <chrono>

int main() 
{ 
    TCPServer mainServer;  
    ProtocolHandler protoclHandler;

    EventManager::getInstance()->addListener(EventManager::EVENT::CREATE_GAME, [&](void* msg) {        
        Game * g = new Game(*reinterpret_cast<std::string*>(msg));        
    });

    EventManager::getInstance()->addListener(EventManager::EVENT::GET_ALL_GAMES, [&](void* sock) {
        for (const auto& g : Game::games) {
            std::string msg = "G-" + g->getInfos();            
            mainServer.sendMessage(msg, *reinterpret_cast<SOCKET*>(sock));
        }
    });


    EventManager::getInstance()->addListener(EventManager::EVENT::PLAYER_CONNECT_OK, [&](void* sock) {                    ;
        mainServer.sendMessage(protoclHandler.getProcotol(ProtocolHandler::PLAYER_CONNECT_OK), *reinterpret_cast<SOCKET*>(sock));       
    });

    EventManager::getInstance()->addListener(EventManager::EVENT::PLAYER_CONNECT_FAIL, [&](void* sock) {;
        mainServer.sendMessage(protoclHandler.getProcotol(ProtocolHandler::PLAYER_CONNECT_FAIL), *reinterpret_cast<SOCKET*>(sock));
    });

    EventManager::getInstance()->addListener(EventManager::EVENT::PLAYER_INSCRIPTION_OK, [&](void* sock) {;
        mainServer.sendMessage(protoclHandler.getProcotol(ProtocolHandler::PLAYER_INSCRIPTION_OK), *reinterpret_cast<SOCKET*>(sock));
    });

    EventManager::getInstance()->addListener(EventManager::EVENT::PLAYER_INSCRIPTION_FAIL, [&](void* sock) {;
        mainServer.sendMessage(protoclHandler.getProcotol(ProtocolHandler::PLAYER_INSCRIPTION_FAIL), *reinterpret_cast<SOCKET*>(sock));
    });


    while(true) { //replace by join
        if(mainServer.clientsNumber > 0) // for
            mainServer.sendMessage("ping", mainServer.m_socketClients[0]);     
        Sleep(1000);
    }
} 
