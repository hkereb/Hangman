#include "Lobby.h"
#include <iostream>

void Lobby::startGame() {
    game.resetGame(roundsAmount, roundDuration, difficulty);
    game.players = players;
    game.startGame();

    std::cout << "Gra rozpoczęta w lobby: " << name << "\n";
}

void Lobby::setOwner() {
    if (!players.empty()) {
        owner = players[0];
        players[0]->isOwner = true;
    }
    else {
        owner = nullptr;
    }
}
