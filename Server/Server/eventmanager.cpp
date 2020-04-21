#include "eventmanager.h"
#include <iostream>

EventManager::EventManager()
{
}

EventManager::~EventManager()
{    
}

void EventManager::addListener(EventManager::EVENT eventName, std::function<void(void*)>&& handler)
{    
    events.insert_or_assign(eventName, handler);
}

void EventManager::triggerEvent(EventManager::EVENT eventName)
{
    if (events[eventName] != nullptr) {
        events[eventName](nullptr);
    }
    else {
        std::cout << "Bad function call: " << eventName << std::endl;
    }
}


void EventManager::triggerEvent(EventManager::EVENT eventName, void* arg)
{      
    if (events[eventName] != nullptr) {
        events[eventName](arg);
    }
    else {
        std::cout << "Bad function call: " << eventName << std::endl;
    }
}
