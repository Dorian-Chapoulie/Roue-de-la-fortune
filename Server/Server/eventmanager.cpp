#include "eventmanager.h"


EventManager::EventManager()
{
}

EventManager::~EventManager()
{    
}

//We had the L-Value function 'handler' to our map, with 'eventName' as key
void EventManager::addListener(EventManager::EVENT eventName, std::function<void(void*)>&& handler)
{
	//If the event is already in our map, whe reafect his value
    auto it = events.find(eventName);
    if(it != events.end()) {
        events[eventName] = handler;
    }else {
    	//Otherwise we insert it in our map
        events.insert(std::make_pair(eventName, handler));
    }
}

//We trigger the event
void EventManager::triggerEvent(EventManager::EVENT eventName)
{
	//If the function linked to the event is correct, we call it
    if (events[eventName] != nullptr) {
        events[eventName](nullptr);
    }
    else {
        //std::cout << "Bad function call: " << eventName << std::endl;
    }
}

//Same but with a parameter
void EventManager::triggerEvent(EventManager::EVENT eventName, void* arg)
{      
    if (events[eventName] != nullptr) {
        events[eventName](arg);
    }
    else {
        //std::cout << "Bad function call: " << eventName << std::endl;
    }
}
