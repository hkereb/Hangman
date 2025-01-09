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

    Game game;

    Lobby() : playersCount(0), difficulty(1), roundsAmount(5), roundDuration(60) {}


    void startGame();
    void setOwner();

};

#endif // LOBBY_H
