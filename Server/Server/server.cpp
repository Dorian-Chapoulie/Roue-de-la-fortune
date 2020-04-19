#include <iostream>
#include <vector>

#include "tcp_server.h"
#include "Game.h"
#include "eventmanager.h"
#include "protocolhandler.h"
#include "Config.h"

#include <chrono>

int main() 
{      
    std::vector<Game*> games;
    EventManager eventManger;
    ProtocolHandler protoclHandler(&eventManger);
    TCPServer mainServer(&protoclHandler);

    eventManger.addListener(EventManager::EVENT::CREATE_GAME, [&](void* msg) {        
        games.push_back(new Game(*reinterpret_cast<std::string*>(msg), Config::getInstance()->basePort + games.size() + 1));
        for (SOCKET s : mainServer.getClients()) {
            mainServer.sendMessage("G-" + games.back()->getInfos(), s);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    });

    eventManger.addListener(EventManager::EVENT::GET_ALL_GAMES, [&](void* sock) {
        for (const auto& g : games) {
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

    eventManger.addListener(eventManger.PLAYER_DISCONNECTED, [&](void* socket) {
       
    });

    //supprimer partie chez le client

    while(true) { //replace by join
        if (games.size() > 0) {
            
            auto time = std::chrono::system_clock::now();          
            std::vector<Game*>::iterator iterator = std::find_if(games.begin(), games.end(), [=](Game* g) {
                std::chrono::duration<double> elapsed_seconds = time - g->getCreatedDate();                
                return elapsed_seconds.count() >= 2.0;
            });

            if (iterator != games.end()) {                
                delete* iterator;
                games.erase(iterator);
            }
        }                
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
} 
