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
	
	//put the money of the player in his bank, then set his money to 0
	void setMoneyInBank();
	
	void clearMoney();
	int getMoney();
	int getBank();

	//Override boolean operator
	bool operator==(Player p);
	
private:
	SOCKET id;
	int money = 0;
	int bank = 0;
	std::string name;
};

