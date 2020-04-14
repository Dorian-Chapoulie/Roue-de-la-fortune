#include "entity/player.h"

Player::Player(std::string& name) : name(name)
{

}

std::string Player::getName() const
{
    return this->name;
}
