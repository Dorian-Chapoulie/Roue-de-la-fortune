#include "eventmanager.h"

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


void EventManager::triggerEvent(EventManager::EVENT eventName, void* arg)
{        
    events[eventName](arg);
}
