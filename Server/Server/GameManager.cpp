#include "GameManager.h"
#include <algorithm>
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

void GameManager::setEventsHandler()
{
	//We set all the events
	this->eventManager->addListener(eventManager->PLAYER_QUICK_RIDDLE, [&](void* data){
        //data = reponse-id
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

		//If the player's response is good
		if(response == currentSentence)
		{
            //If the player is in our player list
			if(it != players->end())
			{
                player = reinterpret_cast<Player*>(*it);
                player->addMoney(QUICK_RIDDLE_MONNEY);

                //We notify every players that the money of the player "player" has changed
                for (const auto* p : *players)
                {
                    SOCKET tmp = p->getId();
                    game->getServer()->sendMessage(protocol_->getSendMoneyProtocol(player), tmp);
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
                
                winnerId = id;
                isQuickRiddleFound = true;
			}                                
		}else //He's wrong
		{
            player = reinterpret_cast<Player*>(*it);
            std::string toSend = player->getName() + " propose: " + response;
			//We notify every players that "player" has proposed something
            for (const auto* p : *players)
            {
                SOCKET tmp = p->getId();
                game->getServer()->sendMessage(protocol_->getServerChatProtocol(toSend), tmp);
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }

            SOCKET sock = id;
            game->getServer()->sendMessage(protocol_->getBadResponseProtocol(), sock);
		}
        mutex->unlock();
	});

	//Same but with a normal enigma
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
                SOCKET tmp = p->getId();
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
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            
        	
            SOCKET sock = id;
            game->getServer()->sendMessage(protocol_->getBadResponseProtocol(), sock);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            game->getServer()->sendMessage(protocol_->getCanPlayProtocol(false), sock);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            restartWithNewPlayer = true;
            waitedTime = WAITING_TIME;
        }
        mutex->unlock();
        });

	//This is called when a player wants to spin the wheel
	//We generate a random number [0; 360[ and send this value to every clients
    this->eventManager->addListener(EventManager::EVENT::SPIN_WHEEL, [&](void*) {
        srand(time(NULL));
        int randomValue = rand() % 360;//TODO: real random

        mutex->lock();
        for (const auto* p : *players)
        {
            SOCKET tmp = p->getId();
            game->getServer()->sendMessage(protocol_->getSpinWheelProtocol(randomValue), tmp);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        mutex->unlock();
        isWheelStartedSpin = true;
    });

	//When the current player's wheel has terminated spinning the wheel, we handle the value
	//Only the current player's send this message because:
	//Some computers are slower that others, that means that some computer will spin the will way faster
	//If we don't do that, we can have a bug: the wheel will spin infinitly
    this->eventManager->addListener(eventManager->WHEEL_VALUE, [&](void* data)
        {
            std::string value = *static_cast<std::string*>(data);
            handleWheelValue(value);
    		isWheelSpinned = true;            
    });

	//When a player propose a letter, we save his choice
    this->eventManager->addListener(eventManager->RECEIVE_LETTER, [&](void* data)
    {
        std::string letter = *static_cast<std::string*>(data);
        playerChoice = letter.at(0);
        playerSentLetter = true;
    });
}

/*
 * This function basicaly read in a file to get a sentence
 * If it is a fast enygma: "quick_riddle.txt"
 * If it is a normal enygma: "riddle.txt"
 */
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

//This function return an array of protocol that contains the char and his position
//If a player propose a 'c' the function return every 'c' and their position in the sentence
//If the array returned is empty, this is a wrong proposition
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

	//We notify every players that this is a "epreuve rapide"
    mutex->lock();
    for(const auto* p : *players)
    {
        SOCKET tmp = p->getId();
        game->getServer()->sendMessage(protocol_->getServerChatProtocol("epreuve rapide."), tmp);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    	//We send to every players the mystery sentence
        game->getServer()->sendMessage(protocol_->getQuickRiddleProtocol(currentSentence), tmp);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    	//We allow every players to play
        game->getServer()->sendMessage(protocol_->getCanPlayProtocol(true), tmp);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    mutex->unlock();

	//This array is all the sent letter's position
    std::vector<int> sentPosition;
    while(!isQuickRiddleFound) {
    	//We send a letter every seconds
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    	//If there is only 1 player left, we stop.
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
        //While our random position is contained in our array, we generate a new random number
        while(it != sentPosition.end()) {
            randomPosition = rand() % currentSentence.length();
            it = std::find(sentPosition.begin(), sentPosition.end(), randomPosition);            
        };

    	//We get the char at the random position
    	//Then whe store our random position in the array "sentPosition"
        char letterToSend = currentSentence.at(randomPosition);
        sentPosition.push_back(randomPosition);

    	//We sent the letter with the position to every players
        mutex->lock();
        for (const auto* p : *players)
        {
            SOCKET tmp = p->getId();
            game->getServer()->sendMessage(protocol_->getSendLetterProtocol(letterToSend, randomPosition), tmp);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        mutex->unlock();                
    }

    //The riddle is finished
    mutex->lock();
    for (auto* p : *players)
    {
        SOCKET tmp = p->getId();
    	//No one can play
        game->getServer()->sendMessage(protocol_->getCanPlayProtocol(false), tmp);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    	//We show the response
        game->getServer()->sendMessage(protocol_->getDisplayResponseProtocol(), tmp);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    mutex->unlock();
	
    return winnerId;
}

int GameManager::sentenceRiddle(int& currentPlayer)
{
	//TODO: refactor as function next block
    this->setCurrentSentence(this->SENTENCE_RIDDLE);
    currentPlayer_ = &currentPlayer;

	//We notify every players of the riddle
    mutex->lock();
    for (const auto* p : *players)
    {
        SOCKET tmp = p->getId();
        game->getServer()->sendMessage(protocol_->getServerChatProtocol("epreuve normale."), tmp);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        game->getServer()->sendMessage(protocol_->getSentenceRiddleProtocol(currentSentence), tmp);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
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

        mutex->lock();
        for (const auto* p : *players)
        {
            SOCKET tmp = p->getId();           

        	//The current player can spin the wheel
            if (currentPlayer == p->getId()) {
                game->getServer()->sendMessage(protocol_->getActivateWheelProtocol(true), tmp);
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                game->getServer()->sendMessage(protocol_->getServerChatProtocol("Vous avez 30 secondes pour tourner la roue."), tmp);
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }else
            {
                game->getServer()->sendMessage(protocol_->getActivateWheelProtocol(false), tmp);
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }

            game->getServer()->sendMessage(protocol_->getCanPlayProtocol(false), tmp);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        mutex->unlock();

    	//While the wheel is not spinned and the riddle is not found
        while(!isWheelSpinned && !isRiddleFound) { 
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        	//We increment the timer => if "waitedTime" == 30s => we set a new current player
        	if(!isWheelStartedSpin) 
				waitedTime += 0.1;

        	//If there is only one player left, we stop.
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
            //If the player is not in our list (deconnected) or the time exceeded 30s, we set a new current player
        	if(it == players->end() || waitedTime >= WAITING_TIME) // 30s
        	{
                waitedTime = 0.0f;
                mutex->lock();
        		if(players->size() > 0)
        		{                    
                    SOCKET tmp = static_cast<SOCKET>(currentPlayer);
        			//curent player cant spin the wheel anymore
                    game->getServer()->sendMessage(protocol_->getActivateWheelProtocol(false), tmp);
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
        			//current player cant play anymore
                    game->getServer()->sendMessage(protocol_->getCanPlayProtocol(false), tmp);
                    std::string newPlayerName;
        			
                    currentPlayer = game->getNextPlayer();
                    newPlayerName = game->getPlayerFromId(currentPlayer)->getName();
        			
                    tmp = static_cast<SOCKET>(currentPlayer);
                   
                    //we notify the new current player
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    game->getServer()->sendMessage(protocol_->getActivateWheelProtocol(true), tmp);
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    game->getServer()->sendMessage(protocol_->getServerChatProtocol("Vous avez 30 secondes pour tourner la roue."), tmp);

                    

                    
                    for (const auto* p : *players)
                    {
                        SOCKET tmp = p->getId();
                        std::this_thread::sleep_for(std::chrono::milliseconds(10));
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
        waitedTime = 0.0f;

    	//while we didn't have processed the wheel value = the player didn't finished spinning the weel
    	//we wait
        mutex->lock();
        int size = players->size();
        mutex->unlock();
    	do
    	{
            mutex->lock();
            size = players->size();
            mutex->unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        } while (!processedWheelValue && size >= 2);
        processedWheelValue = false;

    	//if we must restart with a new current player
        if (restartWithNewPlayer) continue;

    	//The wheel has finished spinning, we have processed the value, let's play !
    	SOCKET s = currentPlayer;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        game->getServer()->sendMessage(protocol_->getCanPlayProtocol(true),  s);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

    	//Same that when we wait for the weel to spin, but now we wait for the player to propose a letter
	    while (!playerSentLetter && !isRiddleFound) {
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
	                std::this_thread::sleep_for(std::chrono::milliseconds(10));
	                std::string newPlayerName;
	                for (const auto* p : *players)
	                {
	                    if (p->getId() != currentPlayer)
	                    {
	                        currentPlayer = p->getId();
	                        newPlayerName = p->getName();
                            ////std::cout << "new player: " << newPlayerName << " :" << std::endl;
	                        break;
	                    }
	                }

	                tmp = static_cast<SOCKET>(currentPlayer);
	                

	                std::this_thread::sleep_for(std::chrono::milliseconds(10));
	                game->getServer()->sendMessage(protocol_->getServerChatProtocol("Vous avez 30 secondes pour proposer une lettre."), tmp);

	                for (const auto* p : *players)
	                {
	                    SOCKET tmp = p->getId();
	                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
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
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
	    game->getServer()->sendMessage(protocol_->getCanPlayProtocol(true), s);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

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
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        mutex->unlock();
    	
	    if(foundLetters.size() <= 0) //si il a rien trouvé
	    {
	    	//He can't play anymore
            game->getServer()->sendMessage(protocol_->getCanPlayProtocol(false), s);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
	    	//We notify that this is a wrong proposition
            game->getServer()->sendMessage(protocol_->getBadResponseProtocol(), s);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
	    	
	        currentPlayer = game->getNextPlayer();
            Player* player = game->getPlayerFromId(currentPlayer);

	    	//We sent a letter with a "-1" position => wrong proposition
            std::string toSend = "W-";
            toSend.push_back(playerChoice);
            toSend += "--1";
	    	
            for (Player* p : *players)
            {
                SOCKET tmp = p->getId();                
                game->getServer()->sendMessage(toSend, tmp);
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
	    	
            mutex->lock();
            for (const auto* p : *players)
            {
                SOCKET tmp = p->getId();            
                game->getServer()->sendMessage(protocol_->getServerChatProtocol(player->getName() + " prend la main !"), tmp);
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            mutex->unlock();
	    	
	    }else 
        {
	    	//We send a protocol for every letter found
            mutex->lock();
            for (std::string s : foundLetters)
            {
                for (Player* p : *players)
                {
                    SOCKET tmp = p->getId();
                    game->getServer()->sendMessage(s, tmp);
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
            }
	    	//we check if this is a voyelle or not
            char choice = foundLetters.at(0).substr(2).at(0);
            Player* player = game->getPlayerFromId(currentPlayer);
            if (!isVoyelle(choice))
            {
                int money = wheelValue * foundLetters.size();                
                player->addMoney(money);                                
            }else
            {
            	//a juicy voyelle is pricyyyyyy
                player->addMoney(-200);
            }

            //We notify that the current player's money has changed
            for (const auto* p : *players)
            {
                SOCKET tmp = p->getId();
                game->getServer()->sendMessage(protocol_->getSendMoneyProtocol(player), tmp);
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            mutex->unlock();
           
	    }
	    foundLetters.clear();
        

    }

	//End of the riddle, no one can play and we display the sentence
    mutex->lock();
    for (auto* p : *players)
    {
        SOCKET tmp = p->getId();
        game->getServer()->sendMessage(protocol_->getCanPlayProtocol(false), tmp);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        game->getServer()->sendMessage(protocol_->getDisplayResponseProtocol(), tmp);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    mutex->unlock();
	
    return winnerId;
}

//Last spin for the winner
//Same has before but we don't wait for a proposition
void GameManager::lastSpin(int& currentPlayer)
{
    //TODO: refactor as function next block
    currentPlayer_ = &currentPlayer;

    mutex->lock();
    for (const auto* p : *players)
    {
        SOCKET tmp = p->getId();
        game->getServer()->sendMessage(protocol_->getServerChatProtocol("Manche du gagnant !"), tmp);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    mutex->unlock();

    isWheelSpinned = false;
    isWheelStartedSpin = false;
    restartWithNewPlayer = false;
    waitedTime = 0.0f;
    wheelValue = 0;


    mutex->lock();
    for (const auto* p : *players)
    {
        SOCKET tmp = p->getId();

        if (currentPlayer == p->getId()) {
            game->getServer()->sendMessage(protocol_->getActivateWheelProtocol(true), tmp);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            game->getServer()->sendMessage(protocol_->getServerChatProtocol("Vous avez 30 secondes pour tourner la roue."), tmp);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        game->getServer()->sendMessage(protocol_->getCanPlayProtocol(false), tmp);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    mutex->unlock();

    while (!isWheelSpinned) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if (!isWheelStartedSpin)
            waitedTime += 0.1;

        mutex->lock();
        if (players->empty())
        {
            mutex->unlock();
            stopGame();
            break;
        }else if(waitedTime >= WAITING_TIME)
        {
            mutex->unlock();
            break;
        }
        mutex->unlock();
    }

    isWheelSpinned = false;
    isWheelSpinned = false;
    waitedTime = 0.0f;

    Player* player = game->getPlayerFromId(currentPlayer);
    if (player != nullptr) {
        player->addMoney(wheelValue);
        SOCKET playerSocket = player->getId();
        game->getServer()->sendMessage(protocol_->getSendMoneyProtocol(player), playerSocket);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

//Function that handle the wheel value
void GameManager::handleWheelValue(std::string value)
{
	//If the value of the wheel is "holdup"
    //set the next player as the current player
    if (value == "HoldUp")
    {
    	//on subtilise fourbement les pépettes des autres joueurs afin de le donner au joueur courant
    	//quel veinard
        mutex->lock();
        Player* player = game->getPlayerFromId(*currentPlayer_);

    	for(Player* p : *players)
    	{
    		if(p != player)
    		{
                player->addMoney(p->getMoney());
                p->clearMoney();
    		}
    	}

        for (Player* p : *players)
        {
            SOCKET tmp = p->getId();
            for (Player* pp : *players)
            {                
                game->getServer()->sendMessage(protocol_->getSendMoneyProtocol(pp), tmp);
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }

        mutex->unlock();
        restartWithNewPlayer = true;
    }
    else if (value == "BankRoute")
    {
    	//clear the money of the current player
    	//set the next player as the current player
        mutex->lock();
        Player* player = game->getPlayerFromId(*currentPlayer_);
        player->clearMoney();

        for (const auto* p : *players)
        {
            SOCKET tmp = p->getId();
            game->getServer()->sendMessage(protocol_->getSendMoneyProtocol(player), tmp);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        mutex->unlock();
        restartWithNewPlayer = true;
        waitedTime = WAITING_TIME;
    }
    else if (value == "Passe")
    {
    	//set the next player as the current player
        waitedTime = WAITING_TIME;
        restartWithNewPlayer = true;
    }
    else
    {
    	//the value is some money
        wheelValue = std::stoi(value);
    }

    processedWheelValue = true;
}

bool GameManager::isVoyelle(char c) const
{
    std::vector<char> voyelles = { 'a', 'e', 'i','o', 'u', 'y' };
    auto it = std::find(voyelles.begin(), voyelles.end(), c);
    return it != voyelles.end();
}
