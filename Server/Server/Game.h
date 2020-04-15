#pragma once
#include "tcp_server.h"
#include <mutex>
#include <vector>
#include "Player.h"
#include "eventmanager.h"

class Game {

public:
	Game(std::string& name);
	~Game();

	std::string getInfos() const;

	static std::vector<Game*> games;

private:
	TCPServer* server = nullptr;
	EventManager eventManager;
	ProtocolHandler* protocol;

	std::string name;
	std::vector<Player*> players;
	std::vector<Player*> spectators;

private:	
	static std::mutex mutex;
};

