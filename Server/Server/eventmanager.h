#ifndef EVENTMANAGER_H
#define EVENTMANAGER_H
#include <functional>
#include <map>

class EventManager
{

public:
    //All events that can be triggered
    enum EVENT {
        PLAYER_CONNECT_OK,
        PLAYER_CONNECT_FAIL,
        PLAYER_INSCRIPTION_OK,
        PLAYER_INSCRIPTION_FAIL,
        CREATE_GAME,
        GET_ALL_GAMES,
        ASK_PSEUDO,
        TCHAT,
        PLAYER_DISCONNECTED,
        PLAYER_QUICK_RIDDLE,
        SPIN_WHEEL,
    	WHEEL_VALUE,
    	RECEIVE_LETTER,
    	PLAYER_SENTENCE_RIDDLE,    	
    };

	//We had a function to our map of events
	//When an event is triggered, the function linked to it will be called
    void addListener(EVENT eventName, std::function<void(void*)>&& handler);

	//We trigger the event (we call his function)
    void triggerEvent(EVENT eventName);
	//Same but with a parameter
	//TODO: refactor void* to union
    void triggerEvent(EVENT eventName, void* msg);

    EventManager();
    ~EventManager();

private:
	//Our map of event - function
    std::map<EVENT, std::function<void(void*)>> events;
};

#endif // EVENTMANAGER_H
