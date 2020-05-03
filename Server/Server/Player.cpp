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

void Player::setMoneyInBank()
{
	bank += money;
	money = 0;
}

void Player::clearMoney()
{
	money = 0;
}

int Player::getMoney()
{
	return money;
}

int Player::getBank()
{
	return bank;
}

bool Player::operator==(Player p)
{
	return p.id == this->id;
}
