#include "Lobby.h"
#include <iostream>

void Lobby::startGame() {
    for (auto& player : players) {
        player.points = 0;
    }

    if (players.size() < 2) {
        std::cout << "Potrzeba przynajmniej 2 graczy w lobby\n";
        return;
    }

    game = Game(roundsAmount, roundDuration, difficulty);
    game.players = players;
    game.startGame();

    std::cout << "Gra rozpoczęta w lobby: " << name << "\n";
}

void Lobby::setOwner() {
    if (!players.empty()) {
        owner = players[0];
        players[0].isOwner = true;
    }
}
