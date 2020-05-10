#include "Game.h"
#include "Config.h"
#include "eventmanager.h"
#include "protocolhandler.h"
#include "GameManager.h"
#include <algorithm>

Game::Game(std::string& name, int port)
{
	this->name = name;
	this->createdDate = std::chrono::system_clock::now();
	this->protocol = new ProtocolHandler(&eventManager);		

	server = new TCPServer(Config::getInstance()->baseIp, port, protocol);

	//We set the events
	eventManager.addListener(EventManager::EVENT::PLAYER_CONNECT_OK, [&](void* sock) {
		//We add a new player
		mutex.lock();
		this->players.push_back(new Player(*reinterpret_cast<SOCKET*>(sock)));
		int size = players.size();
		mutex.unlock();

		//We notify that the connexion is ok
		std::string msg = protocol->getConnectionOKProtocol(std::to_string(*reinterpret_cast<SOCKET*>(sock)));
		this->server->sendMessage(msg, *reinterpret_cast<SOCKET*>(sock));
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		this->server->sendMessage(protocol->getAskPseudoProtocol(), *reinterpret_cast<SOCKET*>(sock));

		//if the player number is 3, we playyyyy
		if(size == 3)
		{
			std::thread threadGame([&]()
			{
					startGame();
					isThreadGameFinished = true;
			});
			threadGame.detach();
		}
	});

	//Broadcast the message to all players
	eventManager.addListener(EventManager::EVENT::TCHAT, [&](void* data) {
		std::string msg = *reinterpret_cast<std::string*>(data);
		mutex.lock();
		for (Player* p : players) {
			SOCKET id = p->getId();
			server->sendMessage(msg, id);
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
		mutex.unlock();
	});

	//When a player is connected to the server, we ask for his pseudo
	eventManager.addListener(EventManager::EVENT::ASK_PSEUDO, [&](void* pseudoAndSocket) {
		std::string msg = *reinterpret_cast<std::string*>(pseudoAndSocket);
		std::string pseudo = msg.substr(0, msg.find('-'));
		std::string id = msg.substr(msg.find('-') + 1, msg.length() - msg.find('-'));

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
				
				this->server->sendMessage(protocol->getNewPlayerProtocol(p2->getName(), std::to_string(p2->getId())), tempId);
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
			}			
		}
		mutex.unlock();
		
	});

	//We handle a disconnection
	eventManager.addListener(EventManager::EVENT::PLAYER_DISCONNECTED, [&](void* socket) {
		mutex.lock();
		auto it = std::find_if(players.begin(), players.end(), [&](Player* p) {
			return p->getId() == *static_cast<SOCKET*>(socket);
		});

		if (it != players.end()) {
			//std::cout << reinterpret_cast<Player*>(*it)->getName() << " s'est deconnecte" << std::endl;
			players.erase(it);
		}

		for (Player* p : players) {
			SOCKET tmp = p->getId();			
			server->sendMessage(protocol->getPlayerDisconnectedProtocol(std::to_string(*static_cast<SOCKET*>(socket))), tmp);
		}
		mutex.unlock();
	});
	
	//We ping player severy seconds, to check is a player is disconnected or not
	threadPingPlayers = new std::thread([&]() { 
		while (pingPlayers){ //TODO bool is partie finished
			mutex.lock();
			for (Player* p : players) {
				SOCKET tempId = p->getId(); //TODO: fix Lvalue and Rvalue by const cast				
				server->sendMessage("0;", tempId);				
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

//Return a player pointer from an id
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

//We get the nex player
int Game::getNextPlayer()
{
	//If the current player is the last of our list, the next player will be the first of our list
	mutex.lock();
	if(currentPlayer == players.back()->getId())
	{
		mutex.unlock();
		return players.at(0)->getId();
	}

	bool setNewPlayer = false;
	int newPlayer = -1;
	//If the current player is not the last of our list
	//The next player will be the player just after the current player
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
	//If we didnt found the next player, we return the first one
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
	//In a game we have 4 rounds, and the last if for the winner
	for (int i = 1; i < 5; i++) {
		int winnerId = gameManager->quickRiddle(); //quick riddle
		handleWinner(winnerId, gameManager->getCurrentSentence()); //we handle the winner

		//If there is 1 or less player we stop
		mutex.lock();
		if(players.size() <= 1)
		{
			gameManager->stopGame();
			mutex.unlock();
			break;
		}
		mutex.unlock();

		//3s pause
		std::this_thread::sleep_for(std::chrono::milliseconds(3000));

		//Sentence riddle
		winnerId = gameManager->sentenceRiddle(currentPlayer);
		//we handle the winner of the riddle
		handleWinner(winnerId, gameManager->getCurrentSentence());
		//we handle a new round
		hanldeNewRound(i);		

		//after a new round, we save the players money
		mutex.lock();
		for (Player* p : players)
		{
			p->setMoneyInBank();
		}
		
		if (players.size() <= 1)
		{
			gameManager->stopGame();
			mutex.unlock();
			break;
		}
		mutex.unlock();
		
		std::this_thread::sleep_for(std::chrono::milliseconds(3000));
	}

	int size = 0;
	mutex.lock();
	size = players.size();
	mutex.unlock();
	//last spin for the winner
	if(size >= 1)
		gameManager->lastSpin(currentPlayer);
	
	mutex.lock();
	if(players.size() <= 0)
	{
		isGameDone = true;
		mutex.unlock();
		return;
	}

	//handle the money
	Player* winner = players.at(0);
	int temp = players.at(0)->getBank();
	for(Player* p : players)
	{
		if(temp < p->getBank())
		{
			temp = p->getBank();
			winner = p;
		}
	}
	mutex.unlock();

	//Notify the winner
	std::string toSend = "Le joueur: <font color=\"Red\"><b>" + winner->getName() + "</b><font color=\"Orange\"> a gagne !";
	mutex.lock();
	for (Player* p : players)
	{
		SOCKET socket = p->getId();
		server->sendMessage(protocol->getServerChatProtocol(toSend), socket);
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	mutex.unlock();

	
	std::this_thread::sleep_for(std::chrono::milliseconds(3000));
	//Notify if the player p has won or loosed
	mutex.lock();
	for (Player* p : players)
	{
		SOCKET socket = p->getId();
		
		if (p == winner) {
			server->sendMessage(protocol->getVictoryProtocol(), socket);
		}else {
			server->sendMessage(protocol->getLooseProtocol(), socket);							
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	mutex.unlock();

	//the game is done and ready to be deleted
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
		//if the winner is correct
		Player* p = nullptr;
		if(it == players.end())
		{
			currentPlayer = getNextPlayer();
			p = getPlayerFromId(currentPlayer);
		}
		else {
			p = reinterpret_cast<Player*>(*it);
		}

		//we notify the clients
		for (Player* s : players)
		{
			SOCKET tmp = s->getId();
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			server->sendMessage(protocol->getServerChatProtocol("Le gagnant est: " + p->getName() + ", la reponse etait: " + sentence + " !"), tmp);
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			server->sendMessage(protocol->getServerChatProtocol(p->getName() + " prend la main."), tmp);
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			server->sendMessage(protocol->getWinnerIdProtocol(p->getId()), tmp);
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
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

		//no one has won
		for (Player* s : players)
		{
			SOCKET tmp = s->getId();
			server->sendMessage(protocol->getServerChatProtocol("Personne n'a gagne, la reponse etait: " + sentence + " !"), tmp);
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			server->sendMessage(protocol->getServerChatProtocol(players.at(0)->getName() + " prend la main."), tmp);
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
		
	}
	mutex.unlock();
}

void Game::hanldeNewRound(int roundNumber)
{
	//we notify the new round
	mutex.lock();
	for (Player* s : players)
	{
		for (Player* p : players) {
			SOCKET tmp = s->getId();
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			server->sendMessage(protocol->getSendMoneyProtocol(s), tmp);
		}
		SOCKET tmp = s->getId();
		server->sendMessage(protocol->getActivateWheelProtocol(false), tmp);
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		server->sendMessage(protocol->getCanPlayProtocol(false), tmp);
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		server->sendMessage(protocol->getNewRoundProtocol(roundNumber), tmp);
		isGameDone = true;;
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
		if(p->getMoney() < temp)
		{
			id = p->getId();
		}
	}
	mutex.unlock();

	return id;
}
