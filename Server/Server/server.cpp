#include <iostream>
#include <vector>

#include "tcp_server.h"
#include "Game.h"
#include "eventmanager.h"
#include "protocolhandler.h"

#include <chrono>

int main() 
{      
    EventManager eventManger;
    ProtocolHandler protoclHandler(&eventManger);
    TCPServer mainServer(&protoclHandler);

    eventManger.addListener(EventManager::EVENT::CREATE_GAME, [&](void* msg) {
        Game * g = new Game(*reinterpret_cast<std::string*>(msg));                  
    });

    eventManger.addListener(EventManager::EVENT::GET_ALL_GAMES, [&](void* sock) {
        for (const auto& g : Game::games) {
            std::string msg = "G-" + g->getInfos();            
            mainServer.sendMessage(msg, *reinterpret_cast<SOCKET*>(sock));
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    });


    eventManger.addListener(EventManager::EVENT::PLAYER_CONNECT_OK, [&](void* sock) {
        std::string msg = protoclHandler.getProcotol(ProtocolHandler::PLAYER_CONNECT_OK) + std::to_string(*reinterpret_cast<SOCKET*>(sock));
        mainServer.sendMessage(msg, *reinterpret_cast<SOCKET*>(sock));
    });

    eventManger.addListener(EventManager::EVENT::PLAYER_CONNECT_FAIL, [&](void* sock) {
        mainServer.sendMessage(protoclHandler.getProcotol(ProtocolHandler::PLAYER_CONNECT_FAIL), *reinterpret_cast<SOCKET*>(sock));
    });

    eventManger.addListener(EventManager::EVENT::PLAYER_INSCRIPTION_OK, [&](void* sock) {
        mainServer.sendMessage(protoclHandler.getProcotol(ProtocolHandler::PLAYER_INSCRIPTION_OK), *reinterpret_cast<SOCKET*>(sock));
    });

    eventManger.addListener(EventManager::EVENT::PLAYER_INSCRIPTION_FAIL, [&](void* sock) {
        mainServer.sendMessage(protoclHandler.getProcotol(ProtocolHandler::PLAYER_INSCRIPTION_FAIL), *reinterpret_cast<SOCKET*>(sock));
    });


    while(true) { //replace by join
        if (mainServer.getClients().size() > 0) {
            for (auto& client : mainServer.getClients()) {
                //mainServer.sendMessage("ping", client);
            }
        }                
        Sleep(1000);
    }
} 
