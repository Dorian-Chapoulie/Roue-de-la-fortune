#pragma once
#include "tcp_server.h"
#include <mutex>
#include <vector>
#include "Player.h"
#include "eventmanager.h"

class Game {

public:
	//We need a name and a port
	//Every game runs with a dedicated server
	Game(std::string& name, int port);
	~Game();

	std::string getInfos();
	std::chrono::system_clock::time_point getCreatedDate() const;

	EventManager* getEventManager();
	ProtocolHandler* getProtocolHandler();
	Player* getPlayerFromId(int id);
	int getNextPlayer();

	bool isJoinable();
	bool isFinished();
	bool isEmpty();
	
	TCPServer* getServer();
private:

	void startGame();
	void handleWinner(int winnerId, std::string sentence);
	void hanldeNewRound(int roundNumber);
	int getLooser();
	
	TCPServer* server = nullptr;
	EventManager eventManager;
	ProtocolHandler* protocol;

	std::thread* threadPingPlayers;

	std::string name;

	std::vector<Player*> players;
	
	std::recursive_mutex mutex;
	std::chrono::system_clock::time_point createdDate;

	int currentPlayer = -1;

	bool pingPlayers = true;
	bool isThreadPingFinished = false;
	bool isGameFinished = false;
	bool isGameDone = false;
	bool isGameStarted = false;
	bool isThreadGameFinished = false;
};

