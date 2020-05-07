#pragma once
#include <string>

#ifdef _WIN32
	#include <winsock2.h>
#else
	typedef int SOCKET;
#endif

class Player
{
public:
	Player(SOCKET id);

	int getId() const;
	std::string getName() const;

	void setName(std::string& name);
	void addMoney(int ammount);
	void setMoneyInBank();
	void clearMoney();
	int getMoney();
	int getBank();

	bool operator==(Player p);
	
private:
	SOCKET id;
	int money = 0;
	int bank = 0;
	std::string name;
};

