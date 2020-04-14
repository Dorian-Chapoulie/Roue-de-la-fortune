#pragma once
#include <string>
class Config {

public:

	static Config* getInstance();


	std::string baseIp = "localhost";
	unsigned int basePort = 25565;


private:
	Config();

private:
	static Config* instance;
};

