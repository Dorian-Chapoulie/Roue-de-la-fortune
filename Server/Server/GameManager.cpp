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
#include <iostream>
void GameManager::setEventsHandler()
{
	this->eventManager->addListener(eventManager->PLAYER_QUICK_RIDDLE, [&](void* data){
        std::string msgAndId = *static_cast<std::string*>(data);
        const std::string response = msgAndId.substr(0, msgAndId.find("-"));
        int id = std::stoi(msgAndId.substr(msgAndId.find("-") + 1, msgAndId.length() - msgAndId.find("-")));
		if(response == currentSentence)
		{
            auto it = std::find_if(players->begin(), players->end(), [&](Player* p)
                {
                    return id == p->getId();
                });
			if(it != players->end())
			{
                Player* p = reinterpret_cast<Player*>(*it);
                p->addMoney(QUICK_RIDDLE_MONNEY);
                SOCKET tmp = p->getId();
                game->getServer()->sendMessage(protocol_->getSendMoneyProtocol(p), tmp);
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}            
            winnerId = id;
            isQuickRiddleFound = true;           
		}else
		{
            SOCKET sock = id;
            game->getServer()->sendMessage(protocol_->getProcotol(protocol_->BAD_RESPONSE), sock);
		}
	});

    this->eventManager->addListener(EventManager::EVENT::SPIN_WHEEL, [&](void*) {
        int randomValue = rand() % 360;//TODO: real random
        mutex->lock();
        for (const auto* p : *players)
        {
            SOCKET tmp = p->getId();
            game->getServer()->sendMessage(protocol_->getSpinWheelProtocol(randomValue), tmp);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        mutex->unlock();
        isWheelStartedSpin = true;
        });
	
    this->eventManager->addListener(eventManager->WHEEL_VALUE, [&](void* data)
        {
            std::string value = *static_cast<std::string*>(data);
            handleWheelValue(value);
    		isWheelSpinned = true;            
    });

    this->eventManager->addListener(eventManager->RECEIVE_LETTER, [&](void* data)
    {
        std::string letter = *static_cast<std::string*>(data);
        playerChoice = letter.at(0);
        playerSentLetter = true;
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

std::vector<std::string> GameManager::getCharFromSentence(char c)
{
    std::vector<std::string> ret;
	for(int i = 0;  i < currentSentence.length(); i++)
	{
		if(currentSentence.at(i) == c)
		{
            ret.push_back(protocol_->getSendLetterProtocol(c, i));
		}
	}
    return ret;
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
    	
    	if(sentPosition.size() == currentSentence.length()) //personne n'a gagn�
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
int GameManager::sentenceRiddle(int& currentPlayer)
{
	//TODO: refactor as function next block
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
            game->getServer()->sendMessage(protocol_->getServerChatProtocol("Vous avez 30 secondes pour tourner la roue."), tmp);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    	
        game->getServer()->sendMessage(protocol_->getCanPlayProtocol(false), tmp);        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    mutex->unlock();
	
    float waitedTime = 0.0f;
    while(!isRiddleFound) {
    	
        while(!isWheelSpinned) { // and if players > 0
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        	if(!isWheelStartedSpin) 
				waitedTime += 0.1;

            auto it = std::find_if(players->begin(), players->end(), [&](Player* p)
                {
                    return p->getId() == currentPlayer;
                });

        	if(it == players->end() || waitedTime >= WAITING_TIME) // 30s
        	{
                waitedTime = 0.0f;
        		if(players->size() > 0)
        		{
                    mutex->lock();
                    SOCKET tmp = static_cast<SOCKET>(currentPlayer);
                    game->getServer()->sendMessage(protocol_->getActivateWheelProtocol(false), tmp);
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    std::string newPlayerName;
        			for(const auto* p : *players)
        			{
        				if(p->getId() != currentPlayer)
        				{
                            currentPlayer = p->getId();
                            newPlayerName = p->getName();
                            break;
        				}
        			}
        			
                    tmp = static_cast<SOCKET>(currentPlayer);
                    mutex->unlock();
        			
                    game->getServer()->sendMessage(protocol_->getActivateWheelProtocol(true), tmp);
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    game->getServer()->sendMessage(protocol_->getServerChatProtocol("Vous avez 30 secondes pour tourner la roue."), tmp);

                    

                    mutex->lock();
                    for (const auto* p : *players)
                    {
                        SOCKET tmp = p->getId();
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                        game->getServer()->sendMessage(protocol_->getServerChatProtocol(newPlayerName + " prend la main !"), tmp);                        
                    }
                    mutex->unlock();

        		}else
        		{
                    break;
        		}
        	}
        }

        isWheelSpinned = false;
        isWheelSpinned = false;
        waitedTime = 0.0f;
    	
    	SOCKET s = currentPlayer;
        game->getServer()->sendMessage(protocol_->getCanPlayProtocol(true),  s);

        while (true) {
            while (!playerSentLetter) { // and if players > 0
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                waitedTime += 0.1;

                auto it = std::find_if(players->begin(), players->end(), [&](Player* p)
                    {
                        return p->getId() == currentPlayer;
                    });

                if (it == players->end() || waitedTime >= WAITING_TIME) // 30s
                {
                    waitedTime = 0.0f;
                    if (players->size() > 0)
                    {
                        mutex->lock();
                        SOCKET tmp = static_cast<SOCKET>(currentPlayer);
                        game->getServer()->sendMessage(protocol_->getCanPlayProtocol(false), tmp);
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                        std::string newPlayerName;
                        for (const auto* p : *players)
                        {
                            if (p->getId() != currentPlayer)
                            {
                                currentPlayer = p->getId();
                                newPlayerName = p->getName();
                                break;
                            }
                        }

                        tmp = static_cast<SOCKET>(currentPlayer);
                        mutex->unlock();

                        game->getServer()->sendMessage(protocol_->getCanPlayProtocol(true), tmp);
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                        game->getServer()->sendMessage(protocol_->getServerChatProtocol("Vous avez 30 secondes pour proposer une lettre."), tmp);


                        mutex->lock();
                        for (const auto* p : *players)
                        {
                            SOCKET tmp = p->getId();
                            std::this_thread::sleep_for(std::chrono::milliseconds(100));
                            game->getServer()->sendMessage(protocol_->getServerChatProtocol(newPlayerName + " prend la main !"), tmp);
                        }
                        mutex->unlock();

                    }
                    else
                    {
                        break;
                    }
                }
            }
            playerSentLetter = false;
            waitedTime = 0.0f;
        	
            s = currentPlayer;
            game->getServer()->sendMessage(protocol_->getCanPlayProtocol(true), s);

            std::vector<std::string> foundLetters = getCharFromSentence(playerChoice);
            for (std::string s : foundLetters)
            {
                for (Player* p : *players)
                {
                    SOCKET tmp = p->getId();
                    game->getServer()->sendMessage(s, tmp);
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            }
        }
        break;
    }

    return -1;
}

void GameManager::handleWheelValue(std::string value)
{
    if (value == "HoldUp")
    {

    }
    else if (value == "BankRoute")
    {

    }
    else if (value == "Passe")
    {

    }
    else
    {
        wheelValue = std::stoi(value);
    }
}
