#pragma once

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
    bool isReadyToPlay;

    Player() {
        this->nick = "";
        this->sockfd = -1;
        this->isOwner = false;
        this->points = 0;
        this->maxLives = 10;
        this->lives = this->maxLives;
        this->lobbyName = "";
        this->failedLetters = {};
        this->isReadyToPlay = false;
    }
};