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
      
}

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
		}else
		{
            SOCKET sock = id;
            game->getServer()->sendMessage(protocol_->getProcotol(protocol_->BAD_RESPONSE), sock);
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
	}else if(type == EnigmaType::SENTENCE_RIDDLE) {
        std::string line;
        std::ifstream myFile(Config::getInstance()->normalRiddleFile);

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

std::string GameManager::getCurrentSentence()
{
    return currentSentence;
}

int GameManager::quickRiddle()
{
    this->setCurrentSentence(this->QUICK_RIDDLE);
    isQuickRiddleFound = false;
	
    mutex->lock();
    for(const auto* p : *players)
    {
        SOCKET tmp = p->getId();
        game->getServer()->sendMessage(protocol_->getServerChatProtocol("epreuve rapide."), tmp);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        game->getServer()->sendMessage(protocol_->getQuickRiddleProtocol(currentSentence), tmp);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));        
        game->getServer()->sendMessage(protocol_->getCanPlayProtocol(true), tmp);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    mutex->unlock();

	
    std::vector<int> sentPosition;
    while(!isQuickRiddleFound) {
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    	
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


    mutex->lock();
    for (const auto* p : *players)
    {
        SOCKET tmp = p->getId();
        game->getServer()->sendMessage(protocol_->getCanPlayProtocol(false), tmp);
        game->getServer()->sendMessage(protocol_->getProcotol(protocol_->DISPLAY_RESPONSE), tmp);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    mutex->unlock();
	
    return winnerId;
}

#include <iostream>
int GameManager::sentenceRiddle(int& currentPlayer, bool& isWheelSpinned)
{
    this->setCurrentSentence(this->SENTENCE_RIDDLE);
    bool isRiddleFound = false;
    mutex->lock();
    for (const auto* p : *players)
    {
        SOCKET tmp = p->getId();
        game->getServer()->sendMessage(protocol_->getServerChatProtocol("epreuve normale."), tmp);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        game->getServer()->sendMessage(protocol_->getSentenceRiddleProtocol(currentSentence), tmp);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if (currentPlayer == p->getId()) {
            game->getServer()->sendMessage(protocol_->getActivateWheelProtocol(true), tmp);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            game->getServer()->sendMessage(protocol_->getCanPlayProtocol(false), tmp);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    mutex->unlock();
	

    while(!isRiddleFound) {
    	
        while(!isWheelSpinned) { // and if players > 0
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            auto it = std::find_if(players->begin(), players->end(), [&](Player* p)
                {
                    return p->getId() == currentPlayer;
                });

        	if(it == players->end())
        	{
        		if(players->size() > 0)
        		{
                    currentPlayer = players->at(0)->getId();
        		}else
        		{
                    break;
        		}
        	}
            std::cout << "waiting for player" << std::endl;
        }
        SOCKET s = currentPlayer;
        game->getServer()->sendMessage(protocol_->getCanPlayProtocol(true),  s);

    	//wait for tempt
    }

    return -1;
}
