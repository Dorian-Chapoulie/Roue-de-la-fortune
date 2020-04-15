#include "entity/player.h"

Player::Player(std::string& name, SOCKET id) : name(name), id(id)
{

}

Player::Player(std::string name) : name(name)
{

}

std::string& Player::getName()
{
    return this->name;
}

SOCKET Player::getId()
{
    return id;
}
