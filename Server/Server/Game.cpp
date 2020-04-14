#include "Game.h"
#include "Config.h"

std::vector<Game*> Game::games;
std::mutex Game::mutex;

#include <iostream>
Game::Game(std::string& name)
{
	this->name = name;
	mutex.lock();
	server = new TCPServer(Config::getInstance()->baseIp, Config::getInstance()->basePort + games.size() + 1);
	games.push_back(this);
	std::cout << "New server '" << name << "'\t " << Config::getInstance()->baseIp << ":" << Config::getInstance()->basePort + games.size() << std::endl;
	mutex.unlock();
}

Game::~Game() {}

std::string Game::getInfos() const
{
	std::string ret = this->name
		+ "-"
		+ std::to_string(this->player.size())
		+ "-"
		+ std::to_string(this->spectators.size())
		+ "-"
		+ server->getIp()
		+ "-"
		+ std::to_string(server->getPort());

	return ret;
}
