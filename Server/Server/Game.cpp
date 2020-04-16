#include "Game.h"
#include "Config.h"
#include "eventmanager.h"
#include "protocolhandler.h"

std::vector<Game*> Game::games;
std::mutex Game::mutex;

#include <iostream>
Game::Game(std::string& name)
{
	this->name = name;
	protocol = new ProtocolHandler(&eventManager);

	mutex.lock();
	server = new TCPServer(Config::getInstance()->baseIp, Config::getInstance()->basePort + games.size() + 1, protocol);
	games.push_back(this);
	std::cout << "New server '" << name << "'\t " << Config::getInstance()->baseIp << ":" << Config::getInstance()->basePort + games.size() << std::endl;
	mutex.unlock();	

	eventManager.addListener(EventManager::EVENT::PLAYER_CONNECT_OK, [&](void* sock) {		
		this->players.push_back(new Player(*reinterpret_cast<SOCKET*>(sock)));
		this->server->sendMessage(protocol->getProcotol(protocol->ASK_PSEUDO), *reinterpret_cast<SOCKET*>(sock));
	});

	eventManager.addListener(EventManager::EVENT::TCHAT, [&](void* data) {
		std::string msg = *reinterpret_cast<std::string*>(data);
		for (Player* p : players) {
			SOCKET id = p->getId();
			server->sendMessage(msg, id);
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	});

	eventManager.addListener(EventManager::EVENT::ASK_PSEUDO, [&](void* pseudoAndSocket) {
		std::string msg = *reinterpret_cast<std::string*>(pseudoAndSocket);
		std::string pseudo = msg.substr(0, msg.find('-'));
		std::string id = msg.substr(msg.find('-') + 1, msg.length() - msg.find('-'));

		std::cout << "New player: " << pseudo << ":" << id << std::endl;

		for (Player* p : players) {
			if (p->getId() == std::stoi(id)) {
				std::cout << "setting '" << pseudo << "' to " << p->getId() << std::endl;
				p->setName(pseudo);
				break;
			}
		}

		for (Player* p : players) {								
			for (Player* p2 : players) {
				SOCKET tempId = p->getId();
				
				this->server->sendMessage(protocol->getProcotol(protocol->NOTIFY_NEW_PLAYER) + "-" + p2->getName() + "-" + std::to_string(p2->getId()), tempId);
				std::this_thread::sleep_for(std::chrono::milliseconds(300));
			}			
		}
	});
}

Game::~Game() {
	this->players.clear();
	this->spectators.clear();
}

std::string Game::getInfos() const
{
	std::string ret = this->name
		+ "-"
		+ std::to_string(this->players.size())
		+ "-"
		+ std::to_string(this->spectators.size())
		+ "-"
		+ server->getIp()
		+ "-"
		+ std::to_string(server->getPort());

	return ret;
}
