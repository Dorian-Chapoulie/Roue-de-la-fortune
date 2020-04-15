#ifndef EVENTMANAGER_H
#define EVENTMANAGER_H
#include <functional>
#include <unordered_map>

class EventManager
{

public:

    enum EVENT {
        PLAYER_CONNECT_OK,
        PLAYER_CONNECT_FAIL,
        PLAYER_INSCRIPTION_OK,
        PLAYER_INSCRIPTION_FAIL,
        CREATE_GAME, 
        GET_ALL_GAMES,
        ASK_PSEUDO,
    };

    void addListener(EVENT eventName, std::function<void(void*)>&& handler);
    void triggerEvent(EVENT eventName);
    void triggerEvent(EVENT eventName, void* msg);

    EventManager();
    ~EventManager();

private:
    std::unordered_map<EVENT, std::function<void(void*)> > events;
};

#endif // EVENTMANAGER_H
