#include "GameManager.h"

#include <fstream>

#include "Config.h"

GameManager::GameManager(std::recursive_mutex* mutex, std::vector<Player*>* players, ProtocolHandler* protocol,
	EventManager* eventManager, Game* game)
{
    this->mutex = mutex;
    this->players = players;
    this->protocol_ = protocol;
    this->eventManager = eventManager;
    this->game = game;

    setEventsHandler();
}

void GameManager::stopGame()
{
    isRiddleFound = true;
    isQuickRiddleFound = true;
}
#include <iostream>
void GameManager::setEventsHandler()
{
	this->eventManager->addListener(eventManager->PLAYER_QUICK_RIDDLE, [&](void* data){
        std::string msgAndId = *static_cast<std::string*>(data);
        const std::string response = msgAndId.substr(0, msgAndId.find("-"));
        int id = std::stoi(msgAndId.substr(msgAndId.find("-") + 1, msgAndId.length() - msgAndId.find("-")));
        Player* player = nullptr;
        if (isQuickRiddleFound) return;
		
        mutex->lock();
        auto it = std::find_if(players->begin(), players->end(), [&](Player* p)
            {
                return id == p->getId();
            });
		
		if(response == currentSentence)
		{

			if(it != players->end())
			{
                player = reinterpret_cast<Player*>(*it);
                player->addMoney(QUICK_RIDDLE_MONNEY);

                
                for (const auto* p : *players)
                {
                    SOCKET tmp = p->getId();
                    game->getServer()->sendMessage(protocol_->getSendMoneyProtocol(player), tmp);
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
                
                winnerId = id;
                isQuickRiddleFound = true;
			}                                
		}else
		{
            player = reinterpret_cast<Player*>(*it);
            std::string toSend = player->getName() + " propose: " + response;
            std::cout << currentSentence << ".\n" << response <<  "." <<std::endl;
            for (const auto* p : *players)
            {
                SOCKET tmp = p->getId();
                game->getServer()->sendMessage(protocol_->getServerChatProtocol(toSend), tmp);
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }

            SOCKET sock = id;
            game->getServer()->sendMessage(protocol_->getProcotol(protocol_->BAD_RESPONSE), sock);
		}
        mutex->unlock();
	});

    this->eventManager->addListener(eventManager->PLAYER_SENTENCE_RIDDLE, [&](void* data) {
        std::string msgAndId = *static_cast<std::string*>(data);
        const std::string response = msgAndId.substr(0, msgAndId.find("-"));    	
        Player* p = nullptr;
    	
        if (isRiddleFound) return;
    	
        mutex->lock();
        int id = std::stoi(msgAndId.substr(msgAndId.find("-") + 1, msgAndId.length() - msgAndId.find("-")));
        auto it = std::find_if(players->begin(), players->end(), [&](Player* p)
            {
                return id == p->getId();
            });
    	
        if (response == currentSentence)
        {            
            if (it != players->end())
            {
                p = reinterpret_cast<Player*>(*it);
                SOCKET tmp = p->getId(); //save monney protocol
                winnerId = id;
                isRiddleFound = true;
            }
        }
        else
        {
            p = reinterpret_cast<Player*>(*it);
            std::string toSend = p->getName() + " propose: " + response;
           
            for (const auto* p : *players)
            {
                SOCKET tmp = p->getId();
                game->getServer()->sendMessage(protocol_->getServerChatProtocol(toSend), tmp);
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            
        	
            SOCKET sock = id;
            game->getServer()->sendMessage(protocol_->getProcotol(protocol_->BAD_RESPONSE), sock);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            game->getServer()->sendMessage(protocol_->getCanPlayProtocol(false), sock);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            restartWithNewPlayer = true;
            waitedTime = WAITING_TIME;
        }
        mutex->unlock();
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
        std::this_thread::sleep_for(std::chrono::milliseconds(500));        
        game->getServer()->sendMessage(protocol_->getCanPlayProtocol(true), tmp);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    mutex->unlock();

	
    std::vector<int> sentPosition;
    while(!isQuickRiddleFound) {
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));

        mutex->lock();
        if (players->size() < 2)
        {
            mutex->unlock();
            stopGame();            
            break;
        }
        mutex->unlock();
    	
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
    for (auto* p : *players)
    {
        SOCKET tmp = p->getId();
        game->getServer()->sendMessage(protocol_->getCanPlayProtocol(false), tmp);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
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
	
    mutex->lock();
    for (const auto* p : *players)
    {
        SOCKET tmp = p->getId();
        game->getServer()->sendMessage(protocol_->getServerChatProtocol("epreuve normale."), tmp);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        game->getServer()->sendMessage(protocol_->getSentenceRiddleProtocol(currentSentence), tmp);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    mutex->unlock();
	
    isRiddleFound = false;
    while(!isRiddleFound) {

        isWheelSpinned = false;
        isWheelStartedSpin = false;
        playerSentLetter = false;
        restartWithNewPlayer = false;
        waitedTime = 0.0f;
        playerChoice = '_';
        wheelValue = 0;        

        std::cout << "restart " << std::endl;

        mutex->lock();
        for (const auto* p : *players)
        {
            SOCKET tmp = p->getId();           
        	
            if (currentPlayer == p->getId()) {
                game->getServer()->sendMessage(protocol_->getActivateWheelProtocol(true), tmp);
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                game->getServer()->sendMessage(protocol_->getServerChatProtocol("Vous avez 30 secondes pour tourner la roue."), tmp);
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }else
            {
                game->getServer()->sendMessage(protocol_->getActivateWheelProtocol(false), tmp);
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }

            game->getServer()->sendMessage(protocol_->getCanPlayProtocol(false), tmp);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        mutex->unlock();
    	
        while(!isWheelSpinned && !isRiddleFound) { // and if players > 0
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        	if(!isWheelStartedSpin) 
				waitedTime += 0.1;

            mutex->lock();
            if (players->size() < 2)
            {
                mutex->unlock();
                stopGame();
                break;
            }
            mutex->unlock();

            auto it = std::find_if(players->begin(), players->end(), [&](Player* p)
                {
                    return p->getId() == currentPlayer;
                });

        	if(it == players->end() || waitedTime >= WAITING_TIME) // 30s
        	{
                waitedTime = 0.0f;
                mutex->lock();
        		if(players->size() > 0)
        		{                    
                    SOCKET tmp = static_cast<SOCKET>(currentPlayer);
                    game->getServer()->sendMessage(protocol_->getActivateWheelProtocol(false), tmp);
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    game->getServer()->sendMessage(protocol_->getCanPlayProtocol(false), tmp);
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
                   

                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    game->getServer()->sendMessage(protocol_->getActivateWheelProtocol(true), tmp);
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    game->getServer()->sendMessage(protocol_->getServerChatProtocol("Vous avez 30 secondes pour tourner la roue."), tmp);

                    

                    
                    for (const auto* p : *players)
                    {
                        SOCKET tmp = p->getId();
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                        game->getServer()->sendMessage(protocol_->getServerChatProtocol(newPlayerName + " prend la main !"), tmp);                        
                    }
                    
                    mutex->unlock();
        		}else
        		{
                    mutex->unlock();
                    return -1;
        		}
        	}
        }

        isWheelSpinned = false;
        isWheelSpinned = false;
        waitedTime = 0.0f;        
    	
    	SOCKET s = currentPlayer;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        game->getServer()->sendMessage(protocol_->getCanPlayProtocol(true),  s);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

       
	    while (!playerSentLetter && !isRiddleFound) { // and if players > 0
	        std::this_thread::sleep_for(std::chrono::milliseconds(100));
	        waitedTime += 0.1;

            mutex->lock();
            if (players->size() < 2)
            {
                mutex->unlock();
                stopGame();                
                break;
            }
            mutex->unlock();

            mutex->lock();
	        auto it = std::find_if(players->begin(), players->end(), [&](Player* p)
	            {
	                return p->getId() == currentPlayer;
	            });

	        if (it == players->end() || waitedTime >= WAITING_TIME) // 30s
	        {
	            waitedTime = 0.0f;
	            if (players->size() > 0)
	            {
                    restartWithNewPlayer = true;
	            	
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
	                

	                std::this_thread::sleep_for(std::chrono::milliseconds(100));
	                game->getServer()->sendMessage(protocol_->getServerChatProtocol("Vous avez 30 secondes pour proposer une lettre."), tmp);

	                for (const auto* p : *players)
	                {
	                    SOCKET tmp = p->getId();
	                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
	                    game->getServer()->sendMessage(protocol_->getServerChatProtocol(newPlayerName + " prend la main !"), tmp);
	                }
	                mutex->unlock();

                    break;
	            }
	            else
	            {
                    mutex->unlock();
	                return -1;
	            }
	        }

            mutex->unlock();
	    }
	    playerSentLetter = false;
	    waitedTime = 0.0f;

        if (restartWithNewPlayer) continue;
	    
	    s = currentPlayer;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
	    game->getServer()->sendMessage(protocol_->getCanPlayProtocol(true), s);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

	    std::vector<std::string> foundLetters = getCharFromSentence(playerChoice);	    

        if (isRiddleFound) break;

        Player* player = game->getPlayerFromId(currentPlayer);
        std::string toSend = player->getName() + " propose: ";
        toSend.push_back(playerChoice);
    	
        mutex->lock();
        for (const auto* p : *players)
        {
            SOCKET tmp = p->getId();
            game->getServer()->sendMessage(protocol_->getServerChatProtocol(toSend), tmp);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        mutex->unlock();
    	
	    if(foundLetters.size() <= 0) //si il a rien trouvé
	    {
            game->getServer()->sendMessage(protocol_->getCanPlayProtocol(false), s);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            game->getServer()->sendMessage(protocol_->getProcotol(protocol_->BAD_RESPONSE), s);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
	    	
	        currentPlayer = game->getNextPlayer();
            Player* player = game->getPlayerFromId(currentPlayer);

            std::string toSend = "W-";
            toSend.push_back(playerChoice);
            toSend += "--1";
	    	
            for (Player* p : *players)
            {
                SOCKET tmp = p->getId();
                
                std::cout << "send: " << toSend  << std::endl;
                game->getServer()->sendMessage(toSend, tmp);
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
	    	
            mutex->lock();
            for (const auto* p : *players)
            {
                SOCKET tmp = p->getId();            
                game->getServer()->sendMessage(protocol_->getServerChatProtocol(player->getName() + " prend la main !"), tmp);
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            mutex->unlock();
	    	
	    }else 
        {
            mutex->lock();
            for (std::string s : foundLetters)
            {
                for (Player* p : *players)
                {
                    SOCKET tmp = p->getId();
                    game->getServer()->sendMessage(s, tmp);
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            }
            char choice = foundLetters.at(0).substr(2).at(0);
            Player* player = game->getPlayerFromId(currentPlayer);
            if (!isVoyelle(choice))
            {
                int money = wheelValue * foundLetters.size();                
                player->addMoney(money);                                
            }else
            {
                player->addMoney(-200);
            }

            
            for (const auto* p : *players)
            {
                SOCKET tmp = p->getId();
                game->getServer()->sendMessage(protocol_->getSendMoneyProtocol(player), tmp);
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            mutex->unlock();
           
	    }
	    foundLetters.clear();
        

    }

    mutex->lock();
    for (auto* p : *players)
    {
        SOCKET tmp = p->getId();
        game->getServer()->sendMessage(protocol_->getCanPlayProtocol(false), tmp);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        game->getServer()->sendMessage(protocol_->getProcotol(protocol_->DISPLAY_RESPONSE), tmp);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    mutex->unlock();
	
    return winnerId;
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

bool GameManager::isVoyelle(char c) const
{
    std::vector<char> voyelles = { 'a', 'e', 'i','o', 'u', 'y' };
    auto it = std::find(voyelles.begin(), voyelles.end(), c);
    return it != voyelles.end();
}
