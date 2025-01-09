#ifndef LOBBY_H
#define LOBBY_H

#include <string>
#include <vector>
#include <map>
#include <ctime>
#include <iostream>
#include <memory>
#include "Game.h"

struct Lobby {
    std::string name;
    std::string password;
    int playersCount;
    std::vector<Player> players;
    Player owner;
    std::map<int, time_t> joinTimes;

    int difficulty;
    int roundsAmount;
    int roundDuration;

    std::unique_ptr<Game> game;

    Lobby() {
        this->name = "";
        this->password = "";
        this->playersCount = 0;
        this->difficulty = 1;
        this->roundsAmount = 5;
        this->roundDuration = 60;
        this->game = std::make_unique<Game>();
    }

    


    void startGame();
    void setOwner();

};

#endif // LOBBY_H
