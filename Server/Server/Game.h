#pragma once
#include "tcp_server.h"
#include <mutex>
#include <vector>
#include "Player.h"
#include "eventmanager.h"

class Game {

public:
	Game(std::string& name, int port);
	~Game();

	std::string getInfos() const;
	std::chrono::system_clock::time_point getCreatedDate() const;

	EventManager* getEventManager();
	ProtocolHandler* getProtocolHandler();
	Player* getPlayerFromId(int id);
	int getNextPlayer();
	
	TCPServer* getServer();
private:

	void startGame();
	void handleWinner(int winnerId, std::string sentence);
	
	TCPServer* server = nullptr;
	EventManager eventManager;
	ProtocolHandler* protocol;

	std::thread* threadPingPlayers;

	std::string name;

	std::vector<Player*> players;
	std::vector<Player*> spectators;
	
	std::mutex mutex;
	std::chrono::system_clock::time_point createdDate;

	int currentPlayer = -1;

	bool pingPlayers = true;
	bool isThreadPingFinished = false;
	bool isGameFinished = false;	
};

