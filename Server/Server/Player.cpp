#include "Player.h"

Player::Player(SOCKET id) : id(id)
{
}

int Player::getId() const
{
	return id;
}

std::string Player::getName() const
{
	return name;
}

void Player::setName(std::string& name)
{
	this->name = name;
}

void Player::addMoney(int ammount)
{
	this->money += ammount;
}

void Player::clearMoney()
{
	money = 0;
}

int Player::getMoney()
{
	return money;
}
