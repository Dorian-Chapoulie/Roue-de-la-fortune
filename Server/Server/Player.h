#pragma once
#include <string>
#include <winsock2.h>

class Player
{
public:
	Player(SOCKET id);

	int getId() const;
	std::string getName() const;

	void setName(std::string& name);
	void addMoney(int ammount);
	void clearMoney();
	int getMoney();

private:
	SOCKET id;
	int money = 0;
	std::string name;
};

