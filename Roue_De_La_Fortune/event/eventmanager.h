#ifndef EVENTMANAGER_H
#define EVENTMANAGER_H
#include <functional>
#include <unordered_map>

class EventManager
{

public:

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
        RECEIVE_LETTER,
        WINNER,
        CAN_PLAY,
        BAD_RESPONSE,
        DISPLAY_RESPONSE,
        SPIN_WHEEL,
    };

    static EventManager* getInstance();

    void addListener(EVENT eventName, std::function<void(void*)>&& handler);
    void triggerEvent(EVENT eventName);
    void triggerEvent(EVENT eventName, std::string msg);
    void unSubsribeEvent(EVENT eventName);

private:
    EventManager();
    ~EventManager();

    std::unordered_map<EVENT, std::function<void(void*)> > events;

    static EventManager* instance;

};

#endif // EVENTMANAGER_H
