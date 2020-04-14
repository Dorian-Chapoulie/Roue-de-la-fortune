#include "Config.h"

Config* Config::instance = nullptr;

Config* Config::getInstance() {

	if (instance == nullptr)
		instance = new Config();

	return instance;
}


Config::Config() {}