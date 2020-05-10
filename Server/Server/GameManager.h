#pragma once
#include "Game.h"

class GameManager
{

public:
	GameManager(std::recursive_mutex* mutex,
		std::vector<Player*>* players,
		ProtocolHandler* protocol,
		EventManager* eventManager,
		Game* game);	

	//Enigme rapide
	int quickRiddle();
	//Enigme normale
	int sentenceRiddle(int& currentPlayer);
	//Last spin for the winner
	void lastSpin(int& currentPlayer);
	//Set the wheel value
	void handleWheelValue(std::string value);

	bool isVoyelle(char c) const;
	void stopGame();
	// returns the mystery sentence
	std::string getCurrentSentence();
private:

	enum EnigmaType { QUICK_RIDDLE, SENTENCE_RIDDLE };			
	
	void setEventsHandler();
	void setCurrentSentence(EnigmaType type);
	std::vector<std::string> getCharFromSentence(char c);

	
	Game* game = nullptr;
	ProtocolHandler* protocol_;
	EventManager* eventManager;
	
	std::vector<Player*>* players;
	std::recursive_mutex* mutex;

	std::string currentSentence;
	std::vector<std::string> usedSentences;
	
	bool isQuickRiddleFound = false;
	bool isRiddleFound = false;
	bool isWheelSpinned = false;
	bool isWheelStartedSpin = false;
	bool playerSentLetter = false;
	bool restartWithNewPlayer = false;
	bool processedWheelValue = false;

	float waitedTime = 0.0f;

	char playerChoice = '_';
	
	int winnerId = -1;
	int wheelValue = 0;
	int* currentPlayer_ = nullptr;
	
	const int WAITING_TIME = 30;
	const int QUICK_RIDDLE_MONNEY = 500;
};

