#include "eventmanager.h"


EventManager::EventManager()
{
}

EventManager::~EventManager()
{    
}

void EventManager::addListener(EventManager::EVENT eventName, std::function<void(void*)>&& handler)
{    
    auto it = events.find(eventName);
    if(it != events.end()) {
        events[eventName] = handler;
    }else {
        events.insert(std::make_pair(eventName, handler));
    }
}

void EventManager::triggerEvent(EventManager::EVENT eventName)
{
    if (events[eventName] != nullptr) {
        events[eventName](nullptr);
    }
    else {
        ////std::cout << "Bad function call: " << eventName << std::endl;
    }
}


void EventManager::triggerEvent(EventManager::EVENT eventName, void* arg)
{      
    if (events[eventName] != nullptr) {
        events[eventName](arg);
    }
    else {
        ////std::cout << "Bad function call: " << eventName << std::endl;
    }
}
