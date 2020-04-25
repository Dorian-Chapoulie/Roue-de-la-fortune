#pragma once
#include <string>
class Config {

public:

	static Config* getInstance();


	std::string baseIp = "192.168.1.50";
	unsigned int basePort = 25565;

	const std::string quickRiddleFile = "quickRiddle.txt";
	const std::string normalRiddleFile = "riddle.txt";

private:
	Config();

private:
	static Config* instance;
};

