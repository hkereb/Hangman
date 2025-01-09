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
    std::vector<Player*> players;
    Player* owner;

    int difficulty;
    int roundsAmount;
    int roundDuration;

    Game game;

    Lobby() :
    name(""),
    password(""),
    playersCount(0),
    players({}),
    owner(nullptr),
    difficulty(1),
    roundsAmount(5),
    roundDuration(60){}

    void startGame();
    void setOwner();
};

#endif // LOBBY_H
