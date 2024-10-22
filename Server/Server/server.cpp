
#include <vector>
#include "tcp_server.h"
#include "Game.h"
#include "eventmanager.h"
#include "protocolhandler.h"
#include "Config.h"
#include <algorithm>
#include <chrono>

int main() 
{      
    std::vector<Game*> games;
    std::recursive_mutex mutex;
    EventManager event_manager;
    ProtocolHandler protocolHandler(&event_manager);
    TCPServer main_server(&protocolHandler);
    int lastUsedPort = Config::getInstance()->basePort;
	
    //We link the lambda with the event 'CREATE_GAME'
	//Everytime the event will be triggered, this function will be executed
    event_manager.addListener(EventManager::EVENT::CREATE_GAME, [&](void* msg) {
        mutex.lock();
    	//we create a new game with a new port
    	//TODO: refactor lastUsedPort++ +1 => ++lastUsedPort
        games.push_back(new Game(*static_cast<std::string*>(msg), lastUsedPort++ + 1));

    	//We notify every clients that a new game is created
        for (SOCKET s : main_server.getClients()) {
            main_server.sendMessage("G-" + games.back()->getInfos() + ';', s);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        mutex.unlock();
    });

    //The client wants the list of disponible games. So we obey
    event_manager.addListener(EventManager::EVENT::GET_ALL_GAMES, [&](void* sock) {
        mutex.lock();
        for (const auto& g : games) {
            if (!g->isJoinable()) continue;
            std::string msg = "G-" + g->getInfos() + ';';            
            main_server.sendMessage(msg, *reinterpret_cast<SOCKET*>(sock));
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        mutex.unlock();
    });

    //The client's connection is fine, so we respond as written in our protocol sheet	
    event_manager.addListener(EventManager::EVENT::PLAYER_CONNECT_OK, [&](void* sock) {
        std::string msg = protocolHandler.getProcotol(ProtocolHandler::PLAYER_CONNECT_OK) + std::to_string(*reinterpret_cast<SOCKET*>(sock)) + ';';        
        main_server.sendMessage(msg, *reinterpret_cast<SOCKET*>(sock));
    });

	//We notify that the client's connection have failed
    event_manager.addListener(EventManager::EVENT::PLAYER_CONNECT_FAIL, [&](void* sock) {
        main_server.sendMessage(protocolHandler.getProcotol(ProtocolHandler::PLAYER_CONNECT_FAIL) + ';', *reinterpret_cast<SOCKET*>(sock));
    });

    event_manager.addListener(EventManager::EVENT::PLAYER_INSCRIPTION_OK, [&](void* sock) {
        main_server.sendMessage(protocolHandler.getProcotol(ProtocolHandler::PLAYER_INSCRIPTION_OK) + ';', *reinterpret_cast<SOCKET*>(sock));
    });

    event_manager.addListener(EventManager::EVENT::PLAYER_INSCRIPTION_FAIL, [&](void* sock) {
        main_server.sendMessage(protocolHandler.getProcotol(ProtocolHandler::PLAYER_INSCRIPTION_FAIL) + ';', *reinterpret_cast<SOCKET*>(sock));
    });

	//If a game is finished, we delete it
    while(true) { 
        mutex.lock();
        if (games.size() > 0) {
            
            auto time = std::chrono::system_clock::now();          
            std::vector<Game*>::iterator iterator = std::find_if(games.begin(), games.end(), [=](Game* g) {
                std::chrono::duration<double> elapsed_seconds = time - g->getCreatedDate();                
                return g->isFinished();
            });

            if (iterator != games.end()) {            	
                delete* iterator;
                games.erase(iterator);
            }
        }
        mutex.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
} 
