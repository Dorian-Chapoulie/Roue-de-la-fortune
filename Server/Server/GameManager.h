#pragma once
#include "Game.h"

class GameManager
{

public:
	GameManager(std::mutex* mutex,
		std::vector<Player*>* players,
		ProtocolHandler* protocol,
		EventManager* eventManager,
		Game* game);	

	int quickRiddle();
private:

	enum EnigmaType { QUICK_RIDDLE, SENTENCE_RIDDLE };		

	void initGame();
	
	void setEventsHandler();
	void setCurrentSentence(EnigmaType type);
		
	void sentenceRiddle();

	
	Game* game = nullptr;
	ProtocolHandler* protocol_;
	EventManager* eventManager;
	
	std::vector<Player*>* players;
	std::mutex* mutex;

	std::string currentSentence;
	std::vector<std::string> usedSentences;
	
	bool isQuickRiddleFound = false;
	int winnerId = -1;
};

