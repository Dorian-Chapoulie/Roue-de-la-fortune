#ifndef WHEELFACTORY_H
#define WHEELFACTORY_H

#include <vector>
#include <string>
#include "graphics/wheel.h"

class WheelFactory
{

public:
    static WheelFactory* getInstance();

    enum WheelType {
        WHEEL_ONE,
    };

    Wheel* getWheel(WheelType type);

    ~WheelFactory();

private:
    WheelFactory();

    static WheelFactory* instance;
    std::vector<std::string> wheelOneCases;

};

#endif // WHEELFACTORY_H
