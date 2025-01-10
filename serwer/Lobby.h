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

    // Move constructor
    Lobby(Lobby&& other) noexcept
        : name(std::move(other.name)),
          password(std::move(other.password)),
          playersCount(other.playersCount),
          players(std::move(other.players)),
          owner(other.owner),
          difficulty(other.difficulty),
          roundsAmount(other.roundsAmount),
          roundDuration(other.roundDuration),
          game(std::move(other.game)) {
        other.owner = nullptr;
    }


    Lobby& operator=(Lobby&& other) noexcept {
        if (this != &other) {
            name = std::move(other.name);
            password = std::move(other.password);
            playersCount = other.playersCount;
            players = std::move(other.players);
            owner = other.owner;
            other.owner = nullptr;
            difficulty = other.difficulty;
            roundsAmount = other.roundsAmount;
            roundDuration = other.roundDuration;
            game = std::move(other.game);
        }
        return *this;
    }

    void startGame();
    void setOwner();
};

#endif // LOBBY_H
