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
    auto it = events.find(eventName);
    if(it != events.end()) {
        events[eventName] = handler;
    }else {
        events.insert(std::make_pair(eventName, handler));
    }
}

void EventManager::triggerEvent(EventManager::EVENT eventName)
{
    if(events[eventName] != nullptr)
        events[eventName](nullptr);
}

void EventManager::triggerEvent(EventManager::EVENT eventName, std::string msg)
{        
    if(events[eventName] != nullptr)
        events[eventName](&msg);    
}

void EventManager::unSubsribeEvent(EventManager::EVENT eventName)
{    
    events[eventName] = nullptr;
}
