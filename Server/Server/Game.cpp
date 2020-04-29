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
		mutex.lock();
		this->players.push_back(new Player(*reinterpret_cast<SOCKET*>(sock)));
		int size = players.size();
		mutex.unlock();
		
		std::string msg = protocol->getProcotol(ProtocolHandler::PLAYER_CONNECT_OK) + std::to_string(*reinterpret_cast<SOCKET*>(sock));
		this->server->sendMessage(msg, *reinterpret_cast<SOCKET*>(sock));
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		this->server->sendMessage(protocol->getProcotol(protocol->ASK_PSEUDO), *reinterpret_cast<SOCKET*>(sock));

		if(size == 2)
		{
			std::thread threadGame([&]()
			{
					startGame();
					isThreadGameFinished = true;
			});
			threadGame.detach();
		}
	});

	eventManager.addListener(EventManager::EVENT::TCHAT, [&](void* data) {
		std::string msg = *reinterpret_cast<std::string*>(data);
		mutex.lock();
		for (Player* p : players) {
			SOCKET id = p->getId();
			server->sendMessage(msg, id);
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
		mutex.unlock();
	});

	eventManager.addListener(EventManager::EVENT::ASK_PSEUDO, [&](void* pseudoAndSocket) {
		std::string msg = *reinterpret_cast<std::string*>(pseudoAndSocket);
		std::string pseudo = msg.substr(0, msg.find('-'));
		std::string id = msg.substr(msg.find('-') + 1, msg.length() - msg.find('-'));

		std::cout << "New player: " << pseudo << ":" << id << std::endl;

		mutex.lock();
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
		mutex.unlock();
		
	});

	eventManager.addListener(EventManager::EVENT::PLAYER_DISCONNECTED, [&](void* socket) {
		mutex.lock();
		auto it = std::find_if(players.begin(), players.end(), [&](Player* p) {
			return p->getId() == *static_cast<SOCKET*>(socket);
		});

		if (it != players.end()) {
			std::cout << reinterpret_cast<Player*>(*it)->getName() << " s'est deconnecte" << std::endl;
			players.erase(it);
		}

		for (Player* p : players) {
			SOCKET tmp = p->getId();			
			server->sendMessage(protocol->getProcotol(protocol->PLAYER_DISCONNECTED) + std::to_string(*static_cast<SOCKET*>(socket)), tmp);
		}
		mutex.unlock();
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
}

Game::~Game() {	
	this->players.clear();
	delete protocol;
	pingPlayers = false;
	while (!isThreadPingFinished) {
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
	}
	while (!isThreadGameFinished && isGameStarted) {
		std::cout << "waiting for game to finish" << std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
	}
	delete server;
	server = nullptr;
	delete threadPingPlayers;
}

std::string Game::getInfos()
{
	mutex.lock();
	std::string ret = this->name
		+ "-"
		+ std::to_string(this->players.size())
		+ "-"
		+ std::to_string(0)
		+ "-"
		+ server->getIp()
		+ "-"
		+ std::to_string(server->getPort());
	mutex.unlock();
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

Player* Game::getPlayerFromId(int id)
{
	mutex.lock();
	auto it = std::find_if(players.begin(), players.end(), [&](Player* p)
		{
			return p->getId() == id;
		});

	if(it != players.end())
	{
		mutex.unlock();
		return reinterpret_cast<Player*>(*it);
	}else
	{
		mutex.unlock();
		return nullptr;
	}
	
}

int Game::getNextPlayer()
{
	mutex.lock();
	if(currentPlayer == players.at(players.size() - 1)->getId())
	{
		mutex.unlock();
		return players.at(0)->getId();
	}

	bool setNewPlayer = false;
	int newPlayer = -1;
	for(Player* p : players)
	{
		if(p->getId() == currentPlayer)
		{
			setNewPlayer = true;
			continue;
		}

		if(setNewPlayer)
		{
			newPlayer = p->getId();
			break;
		}
	}

	if(newPlayer == -1)
	{
		newPlayer = players.at(0)->getId();
	}
	mutex.unlock();
	return newPlayer;
}

bool Game::isJoinable()
{
	return !isGameStarted;
}

bool Game::isFinished()
{
	return isGameFinished;
}

bool Game::isEmpty()
{
	int size = 0;
	mutex.lock();
	size = players.size();
	mutex.unlock();

	return size == 0;
}

TCPServer* Game::getServer()
{
	return server;
}

void Game::startGame()
{
	isGameStarted = true;
	
	mutex.lock();
	GameManager* gameManager = new GameManager(&mutex, &players, protocol, &eventManager, this);	
	mutex.unlock();

	for (int i = 1; i < 5; i++) {
		int winnerId = gameManager->quickRiddle();
		handleWinner(winnerId, gameManager->getCurrentSentence());

		mutex.lock();
		if(players.size() <= 1)
		{
			gameManager->stopGame();
			mutex.unlock();
			break;
		}
		mutex.unlock();
		
		std::this_thread::sleep_for(std::chrono::milliseconds(3000));

		winnerId = gameManager->sentenceRiddle(currentPlayer);
		handleWinner(winnerId, gameManager->getCurrentSentence());
		hanldeNewRound(i);

		mutex.lock();
		if (i < 4) {
			if (players.size() > 1) {
				auto it = std::find_if(players.begin(), players.end(), [&](Player* p)
					{
						return p->getId() == getLooser();
					});
				Player* looser = reinterpret_cast<Player*>(*it);
				std::string toSend = "Le joueur: <font color=\"Red\"><b>" + looser->getName() + "</b><font color=\"Orange\"> a perdu !";


				for (Player* s : players)
				{
					SOCKET tmp = s->getId();
					server->sendMessage(protocol->getServerChatProtocol(toSend), tmp);
					std::this_thread::sleep_for(std::chrono::milliseconds(100));
					s->clearMoney();
				}


				SOCKET socket = looser->getId();
				server->sendMessage(protocol->getProcotol(protocol->LOOSE), socket);
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				getServer()->disconnectClient(looser->getId());
				players.erase(it);
			}
			else {
				gameManager->stopGame();
				mutex.unlock();
				break;
			}
		}

		mutex.unlock();
		
		std::this_thread::sleep_for(std::chrono::milliseconds(3000));
	}

	mutex.lock();
	SOCKET socket = players.at(0)->getId();
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	server->sendMessage(protocol->getProcotol(protocol->VICTORY), socket);
	server->disconnectClient(players.at(0)->getId());
	mutex.unlock();
	isGameDone = true;
}

void Game::handleWinner(int winnerId, std::string sentence)
{
	mutex.lock();
	int size = players.size();
	
	
	if (winnerId != -1 && size > 0) {

		auto it = std::find_if(players.begin(), players.end(), [&](Player* p)
			{
				return p->getId() == winnerId;
			});
		currentPlayer = winnerId;

		Player* p = nullptr;
		if(it == players.end())
		{
			currentPlayer = getNextPlayer();
			p = getPlayerFromId(currentPlayer);
		}
		else {
			p = reinterpret_cast<Player*>(*it);
		}

		
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
		

	}
	else {
		if (players.size() > 0) {
			currentPlayer = players.at(0)->getId();
		}
		else
		{
			currentPlayer = -1;
		}

		
		for (Player* s : players)
		{
			SOCKET tmp = s->getId();
			server->sendMessage(protocol->getServerChatProtocol("Personne n'a gagne, la reponse etait: " + sentence + " !"), tmp);
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			server->sendMessage(protocol->getServerChatProtocol(players.at(0)->getName() + " prend la main."), tmp);
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
		
	}
	mutex.unlock();
}

void Game::hanldeNewRound(int roundNumber)
{
	mutex.lock();
	for (Player* s : players)
	{
		SOCKET tmp = s->getId();
		server->sendMessage(protocol->getActivateWheelProtocol(false), tmp);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		server->sendMessage(protocol->getCanPlayProtocol(false), tmp);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		server->sendMessage(protocol->getSendMoneyProtocol(s), tmp);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		server->sendMessage(protocol->getNewRoundProtocol(roundNumber), tmp);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));		
	}
	mutex.unlock();
}

int Game::getLooser()
{
	mutex.lock();
	int temp = players.at(0)->getMoney();
	int id = players.at(0)->getId();
	for(Player* p : players)
	{
		std::cout << p->getName() << ": " << p->getMoney() << std::endl;
		if(p->getMoney() < temp)
		{
			id = p->getId();
		}
	}
	mutex.unlock();

	return id;
}
