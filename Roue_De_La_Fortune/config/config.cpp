#include "config.h"

Config* Config::config = nullptr;

Config* Config::getInstance() {
    if(config == nullptr)
        config = new Config();
    return config;
}

Config::Config()
{

}
