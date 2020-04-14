#include "event/eventmanager.h"
#include "ui/connection.h"

EventManager* EventManager::instance = nullptr;

EventManager* EventManager::getInstance() {
    if(instance == nullptr)
        instance = new EventManager();
    return instance;
}


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
    events[eventName](nullptr);
}

#include <iostream>
void EventManager::triggerEvent(EventManager::EVENT eventName, std::string msg)
{    
    events[eventName](&msg);
}
