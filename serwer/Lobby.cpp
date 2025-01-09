#include "Lobby.h"
#include <iostream>

void Lobby::startGame() {
    for (auto& player : players) {
        player.points = 0;
    }

    game = std::make_unique<Game>(roundsAmount, roundDuration, difficulty);
    game->players = players;
    game->startGame();

    std::cout << "Gra rozpoczęta w lobby: " << name << "\n";
}

void Lobby::setOwner() {
    owner = players[0];
    players[0].isOwner = true;
}

