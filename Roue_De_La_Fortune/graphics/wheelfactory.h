#ifndef WHEELFACTORY_H
#define WHEELFACTORY_H

#include <vector>
#include <string>
#include "graphics/wheel.h"

//Wheel factory that will give a certain instance of wheel
//There is 1 different wheel for every round
class WheelFactory
{

public:
    static WheelFactory* getInstance();

    enum WheelType {
        WHEEL_ONE,
        WHEEl_TWO,
        WHEEL_THREE,
        WHEEL_FOUR,
        WHEEL_WINNER,
    };

    Wheel* getWheel(WheelType type);

    ~WheelFactory();

private:
    WheelFactory();

    static WheelFactory* instance;
    std::vector<std::string> wheelOneCases;
    std::vector<std::string> wheelTwoCases;
    std::vector<std::string> wheelThreeCases;
    std::vector<std::string> wheelFourCases;
    std::vector<std::string> wheelWinnerCases;

};

#endif // WHEELFACTORY_H
