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

void Player::setName(std::string name)
{
    this->name = name;
}

void Player::setMoney(int ammount)
{
    this->money = ammount;
}

void Player::buyVoyelle()
{
    this->money -= 200;
}

void Player::setId(int id)
{
    this->id = id;
}

int Player::getMoney()
{
    return money;
}
