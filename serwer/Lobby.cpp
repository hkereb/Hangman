#include "Lobby.h"
#include <iostream>

void Lobby::startGame() {
    for (auto& player : players) {
        player->points = 0;
    }

    game.resetGame(roundsAmount, roundDuration, difficulty);
    game.players = players;
    game.startGame();

    std::cout << "Gra rozpoczęta w lobby: " << name << "\n";
}

void Lobby::setOwner() {
    if (!players.empty()) { // Sprawdzamy, czy lista graczy nie jest pusta
        owner = players[0]; // Przypisanie wskaźnika na właściciela
        players[0]->isOwner = true; // Dostęp do członka przez wskaźnik
    }
    else {
        owner = nullptr;
    }
}
