#include "Game.h"
#include "Config.h"
#include "eventmanager.h"
#include "protocolhandler.h"
#include "GameManager.h"
#include <iostream>

Game::Game(std::string& name, int port)
{
	this->name = name;
	this->createdDate = std::chrono::system_clock::now();
	this->protocol = new ProtocolHandler(&eventManager);		

	server = new TCPServer(Config::getInstance()->baseIp, port, protocol);
	std::cout << "New server '" << name << "'\t " << Config::getInstance()->baseIp << ":" << port << std::endl;

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
				p->setName(pseudo);
				break;
			}
		}

		for (Player* p : players) {								
			for (Player* p2 : players) {
				SOCKET tempId = p->getId(); //TODO: fix Lvalue and Rvalue by const cast
				
				this->server->sendMessage(protocol->getProcotol(protocol->NOTIFY_NEW_PLAYER) + "-" + p2->getName() + "-" + std::to_string(p2->getId()), tempId);
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
			}			
		}
		
	});

	eventManager.addListener(eventManager.PLAYER_DISCONNECTED, [&](void* socket) {
		auto it = std::find_if(players.begin(), players.end(), [&](Player* p) {
			return p->getId() == *static_cast<SOCKET*>(socket);
		});

		if (it != players.end()) {
			players.erase(it);
		}

		for (Player* p : players) {
			SOCKET tmp = p->getId();			
			server->sendMessage(protocol->getProcotol(protocol->PLAYER_DISCONNECTED) + std::to_string(*static_cast<SOCKET*>(socket)), tmp);
		}
	});

	threadPingPlayers = new std::thread([&]() { 
		while (pingPlayers){ //TODO bool is partie finished
			mutex.lock();
			for (Player* p : players) {
				SOCKET tempId = p->getId(); //TODO: fix Lvalue and Rvalue by const cast				
				server->sendMessage("0", tempId);				
				std::this_thread::sleep_for(std::chrono::milliseconds(200));
			}
			mutex.unlock();
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		}
		isThreadPingFinished = true;
	});
	threadPingPlayers->detach();

	std::thread treadStartGame([&]()
		{
			while(players.size() < 1)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}
			startGame();
		});
	treadStartGame.detach();
}

Game::~Game() {	
	this->players.clear();
	this->spectators.clear();
	delete protocol;
	pingPlayers = false;
	while (!isThreadPingFinished) {
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
	}	
	delete server;
	server = nullptr;	
	std::cout << "Game delete ok" << std::endl;
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

std::chrono::system_clock::time_point Game::getCreatedDate() const
{
	return createdDate;
}

EventManager* Game::getEventManager()
{
	return &eventManager;
}

ProtocolHandler* Game::getProtocolHandler()
{
	return protocol;
}

TCPServer* Game::getServer()
{
	return server;
}

void Game::startGame()
{
	GameManager* gameManager = new GameManager(&mutex, &players, protocol, &eventManager, this);	

	
	int winnerId = gameManager->quickRiddle();
	handleWinner(winnerId, gameManager->getCurrentSentence());
	
	std::this_thread::sleep_for(std::chrono::milliseconds(3000));
	winnerId = gameManager->quickRiddle();
	handleWinner(winnerId, gameManager->getCurrentSentence());
}

void Game::handleWinner(int winnerId, std::string sentence)
{

	if (winnerId != -1) {

		auto it = std::find_if(players.begin(), players.end(), [&](Player* p)
			{
				return p->getId() == winnerId;
			});
		currentPlayer = winnerId;

		Player* p = reinterpret_cast<Player*>(*it);

		mutex.lock();
		for (Player* s : players)
		{
			SOCKET tmp = s->getId();
			server->sendMessage(protocol->getServerChatProtocol("Le gagnant est: " + p->getName() + ", la reponse etait: " + sentence + " !"), tmp);
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			server->sendMessage(protocol->getServerChatProtocol(p->getName() + " prend la main."), tmp);
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			server->sendMessage(protocol->getWinnerIdProtocol(p->getId()), tmp);
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
		mutex.unlock();

	}
	else {
		if (players.size() > 0) {
			currentPlayer = players.at(0)->getId();
		}
		else
		{
			currentPlayer = -1;
		}

		mutex.lock();
		for (Player* s : players)
		{
			SOCKET tmp = s->getId();
			server->sendMessage(protocol->getServerChatProtocol("Personne n'a gagne, la reponse etait: " + sentence + " !"), tmp);
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			server->sendMessage(protocol->getServerChatProtocol(players.at(0)->getName() + " prend la main."), tmp);
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
		mutex.unlock();
	}
}
