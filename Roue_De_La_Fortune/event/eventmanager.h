#ifndef EVENTMANAGER_H
#define EVENTMANAGER_H
#include <functional>
#include <map>
#include <string>

class EventManager
{

public:
    //Every events
    enum EVENT {        
        CONNEXION_SUCCESS,
        CONNEXION_FAILURE,
        INSCRIPTION_SUCCESS,
        INSCRIPTION_FAILURE,
        GAMES_LIST,
        ASK_PSEUDO,
        NEW_PLAYER,
        TCHAT,
        PLAYER_DISCONNECT,
        RECEIVE_QUICK_RIDDLE,
        RECEIVE_SENTENCE_RIDDLE,
        RECEIVE_LETTER,
        WINNER,
        CAN_PLAY,
        BAD_RESPONSE,
        DISPLAY_RESPONSE,
        SPIN_WHEEL,
        ENABLE_WHEEL,
        PLAYER_MONEY,
        NEW_ROUND,
        LOOSE,
        VICTORY,
    };

    static EventManager* getInstance();

    //Add a function to an event
    void addListener(EVENT eventName, std::function<void(void*)>&& handler);
    //We can trigger an event with or withour a parameter
    void triggerEvent(EVENT eventName);
    void triggerEvent(EVENT eventName, std::string msg);
    //We can unsubribe an event
    //If an unsubscribed event is triggered, nothing happens
    void unSubsribeEvent(EVENT eventName);

private:
    EventManager();
    ~EventManager();

    std::map<EVENT, std::function<void(void*)> > events;

    static EventManager* instance;

};

#endif // EVENTMANAGER_H
