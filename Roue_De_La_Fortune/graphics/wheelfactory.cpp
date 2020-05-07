#include "wheelfactory.h"
#include "config/config.h"

WheelFactory* WheelFactory::instance = nullptr;

WheelFactory* WheelFactory::getInstance() {
    if(instance == nullptr)
        instance = new WheelFactory();
    return instance;
}

WheelFactory::WheelFactory()
{
    wheelOneCases.reserve(24);
    wheelOneCases.push_back("100");
    wheelOneCases.push_back("350");
    wheelOneCases.push_back("250");
    wheelOneCases.push_back("100");
    wheelOneCases.push_back("1000");
    wheelOneCases.push_back("HoldUp");
    wheelOneCases.push_back("150");
    wheelOneCases.push_back("250");
    wheelOneCases.push_back("500");
    wheelOneCases.push_back("BankRoute");
    wheelOneCases.push_back("1500");
    wheelOneCases.push_back("150");
    wheelOneCases.push_back("250");
    wheelOneCases.push_back("Passe");
    wheelOneCases.push_back("400");
    wheelOneCases.push_back("2000");
    wheelOneCases.push_back("100");
    wheelOneCases.push_back("0");
    wheelOneCases.push_back("150");
    wheelOneCases.push_back("250");
    wheelOneCases.push_back("300");
    wheelOneCases.push_back("150");
    wheelOneCases.push_back("200");
    wheelOneCases.push_back("BankRoute");

    wheelTwoCases.reserve(24);
    wheelTwoCases.push_back("100");
    wheelTwoCases.push_back("350");
    wheelTwoCases.push_back("250");
    wheelTwoCases.push_back("100");
    wheelTwoCases.push_back("1000");
    wheelTwoCases.push_back("HoldUp");
    wheelTwoCases.push_back("150");
    wheelTwoCases.push_back("250");
    wheelTwoCases.push_back("500");
    wheelTwoCases.push_back("BankRoute");
    wheelTwoCases.push_back("1500");
    wheelTwoCases.push_back("150");
    wheelTwoCases.push_back("250");
    wheelTwoCases.push_back("Passe");
    wheelTwoCases.push_back("400");
    wheelTwoCases.push_back("2000");
    wheelTwoCases.push_back("BankRoute");
    wheelTwoCases.push_back("0");
    wheelTwoCases.push_back("150");
    wheelTwoCases.push_back("250");
    wheelTwoCases.push_back("300");
    wheelTwoCases.push_back("1000");
    wheelTwoCases.push_back("200");
    wheelTwoCases.push_back("BankRoute");

    wheelThreeCases.reserve(24);
    wheelThreeCases.push_back("100");
    wheelThreeCases.push_back("350");
    wheelThreeCases.push_back("250");
    wheelThreeCases.push_back("100");
    wheelThreeCases.push_back("1000");
    wheelThreeCases.push_back("HoldUp");
    wheelThreeCases.push_back("150");
    wheelThreeCases.push_back("250");
    wheelThreeCases.push_back("500");
    wheelThreeCases.push_back("BankRoute");
    wheelThreeCases.push_back("1500");
    wheelThreeCases.push_back("150");
    wheelThreeCases.push_back("250");
    wheelThreeCases.push_back("Passe");
    wheelThreeCases.push_back("400");
    wheelThreeCases.push_back("2000");
    wheelThreeCases.push_back("BankRoute");
    wheelThreeCases.push_back("HoldUp");
    wheelThreeCases.push_back("150");
    wheelThreeCases.push_back("250");
    wheelThreeCases.push_back("300");
    wheelThreeCases.push_back("1000");
    wheelThreeCases.push_back("200");
    wheelThreeCases.push_back("BankRoute");

    wheelFourCases.reserve(24);
    wheelFourCases.push_back("100");
    wheelFourCases.push_back("350");
    wheelFourCases.push_back("250");
    wheelFourCases.push_back("2000");
    wheelFourCases.push_back("1000");
    wheelFourCases.push_back("HoldUp");
    wheelFourCases.push_back("150");
    wheelFourCases.push_back("250");
    wheelFourCases.push_back("500");
    wheelFourCases.push_back("BankRoute");
    wheelFourCases.push_back("1500");
    wheelFourCases.push_back("150");
    wheelFourCases.push_back("250");
    wheelFourCases.push_back("Passe");
    wheelFourCases.push_back("400");
    wheelFourCases.push_back("2000");
    wheelFourCases.push_back("BankRoute");
    wheelFourCases.push_back("HoldUp");
    wheelFourCases.push_back("150");
    wheelFourCases.push_back("250");
    wheelFourCases.push_back("300");
    wheelFourCases.push_back("1000");
    wheelFourCases.push_back("200");
    wheelFourCases.push_back("BankRoute");

    wheelWinnerCases.reserve(24);
    wheelWinnerCases.push_back("100");
    wheelWinnerCases.push_back("200");
    wheelWinnerCases.push_back("300");
    wheelWinnerCases.push_back("400");
    wheelWinnerCases.push_back("500");
    wheelWinnerCases.push_back("600");
    wheelWinnerCases.push_back("700");
    wheelWinnerCases.push_back("800");
    wheelWinnerCases.push_back("900");
    wheelWinnerCases.push_back("1000");
    wheelWinnerCases.push_back("1100");
    wheelWinnerCases.push_back("1200");
    wheelWinnerCases.push_back("1300");
    wheelWinnerCases.push_back("1400");
    wheelWinnerCases.push_back("1500");
    wheelWinnerCases.push_back("1600");
    wheelWinnerCases.push_back("1700");
    wheelWinnerCases.push_back("1800");
    wheelWinnerCases.push_back("1900");
    wheelWinnerCases.push_back("2000");
    wheelWinnerCases.push_back("2100");
    wheelWinnerCases.push_back("2200");
    wheelWinnerCases.push_back("2300");
    wheelWinnerCases.push_back("2400");
}

WheelFactory::~WheelFactory()
{
    delete instance;
}

Wheel* WheelFactory::getWheel(WheelType type) {
    std::string basePath = Config::getInstance()->baseRessourcesPath
            + "/"
            + Config::getInstance()->imageFolder;

    switch (type) {
        case WHEEL_ONE:
            return new Wheel(basePath + "/roue1.png", 24, wheelOneCases);
        break;
        case WHEEl_TWO:
            return new Wheel(basePath + "/roue2.png", 24, wheelTwoCases);
        break;
        case WHEEL_THREE:
            return new Wheel(basePath + "/roue3.png", 24, wheelThreeCases);
        break;
        case WHEEL_FOUR:
            return new Wheel(basePath + "/roue4.png", 24, wheelFourCases);
        break;
        case WHEEL_WINNER:
            return new Wheel(basePath + "/roue5.png", 24, wheelWinnerCases);
        break;
        default:
            return nullptr;
        break;
    };
}
