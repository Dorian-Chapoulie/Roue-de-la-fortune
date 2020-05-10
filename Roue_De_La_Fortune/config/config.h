#ifndef CONFIG_H
#define CONFIG_H
#include <string>

class Config {

public:

    static Config* getInstance();
    const unsigned int baseServerPort = 25565;
    std::string baseServerIP = "51.83.70.16";
    const std::string baseRessourcesPath = "ressources";
    const std::string imageFolder = "images";
    const std::string soundFolder = "sounds";

private:
    Config();
    static Config* config;
};

#endif // CONFIG_H
