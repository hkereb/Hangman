#ifndef PLAYER_H
#define PLAYER_H

#include <string>
#include <vector>

struct Player {
    int sockfd;
    std::string nick;
    std::string lobbyName;

    bool isOwner = false;

    int points;
    int lives;
    int maxLives;
    std::vector<char> failedLetters;

    Player() {
        this->nick = "";
        this->sockfd = -1; // Initialize to -1 instead of NULL for better clarity
        this->isOwner = false;
        this->points = 0;
        this->maxLives = 5;
        this->lives = this->maxLives;
        this->lobbyName = "";
        this->failedLetters = {};
    }
};

#endif // PLAYER_H