#include "GameManager.h"

#include <fstream>

#include "Config.h"

GameManager::GameManager(std::mutex* mutex, std::vector<Player*>* players, ProtocolHandler* protocol,
	EventManager* eventManager, Game* game)
{
    this->mutex = mutex;
    this->players = players;
    this->protocol_ = protocol;
    this->eventManager = eventManager;
    this->game = game;

    setEventsHandler();
    initGame();
}

void GameManager::initGame()
{
    this->setCurrentSentence(this->QUICK_RIDDLE);    
}

#include <iostream>
void GameManager::setEventsHandler()
{

	this->eventManager->addListener(eventManager->PLAYER_QUICK_RIDDLE, [&](void* data){
        std::string msgAndId = *static_cast<std::string*>(data);
        const std::string response = msgAndId.substr(0, msgAndId.find("-"));
        int id = std::stoi(msgAndId.substr(msgAndId.find("-") + 1, msgAndId.length() - msgAndId.find("-")));
		if(response == currentSentence)
		{
            winnerId = id;
            isQuickRiddleFound = true;           
		}
	});
}

void GameManager::setCurrentSentence(EnigmaType type)
{
	if(type == EnigmaType::QUICK_RIDDLE)
	{
        std::string line;
        std::ifstream myFile(Config::getInstance()->quickRiddleFile);

        if (myFile.is_open())
        {
            while (getline(myFile, line))
            {
                auto it = std::find(usedSentences.begin(), usedSentences.end(), line);
                if (it == usedSentences.end())
                {
                    currentSentence = line;
                    usedSentences.push_back(line);
                    break;
                }
            }
            myFile.close();
        }
	}
}

int GameManager::quickRiddle()
{

    mutex->lock();
    for(const auto* p : *players)
    {
        SOCKET tmp = p->getId();
        game->getServer()->sendMessage(protocol_->getQuickRiddleProtocol(currentSentence), tmp);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    mutex->unlock();

	
    std::vector<int> sentPosition;
    while(!isQuickRiddleFound) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    	
    	if(sentPosition.size() == currentSentence.length()) //personne n'a gagné
    	{
            winnerId = -1;
            break;
    	}

        int randomPosition = rand() % currentSentence.length();
        auto it = std::find(sentPosition.begin(), sentPosition.end(), randomPosition);

        while(it != sentPosition.end()) {
            randomPosition = rand() % currentSentence.length();
            it = std::find(sentPosition.begin(), sentPosition.end(), randomPosition);            
        };
    	
        char letterToSend = currentSentence.at(randomPosition);
        sentPosition.push_back(randomPosition);

        mutex->lock();
        for (const auto* p : *players)
        {
            SOCKET tmp = p->getId();
            game->getServer()->sendMessage(protocol_->getSendLetterProtocol(letterToSend, randomPosition), tmp);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        mutex->unlock();                
    }

    return winnerId;
}

void GameManager::sentenceRiddle()
{
}
