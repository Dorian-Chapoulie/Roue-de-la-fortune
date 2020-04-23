#include "wheelfactory.h"

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
}

WheelFactory::~WheelFactory()
{
    delete instance;
}

#include <iostream>
Wheel* WheelFactory::getWheel(WheelType type) {
    switch (type) {
        case WHEEL_ONE:
            return new Wheel("roue1.png", 24, wheelOneCases);
        break;
    };
}
