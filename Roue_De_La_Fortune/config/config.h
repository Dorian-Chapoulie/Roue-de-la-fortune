#ifndef CONFIG_H
#define CONFIG_H
#include <string>

class Config {

public:

    static Config* getInstance();
    const unsigned int baseServerPort = 25565;
    const std::string baseServerIP = "localhost";

private:
    Config();
    static Config* config;
};

#endif // CONFIG_H
