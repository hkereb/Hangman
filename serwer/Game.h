#ifndef GAME_H
#define GAME_H

#include <string>
#include <vector>
#include <chrono>
#include <algorithm>
#include <iostream>
#include <random>
#include <atomic>
#include <thread>
#include "Player.h"
#include "helpers.h"

namespace Levels {
    constexpr int EASY = 1;
    constexpr int MEDIUM = 2;
    constexpr int HARD = 3;
}

struct Game {
    std::vector<std::string> wordList;
    std::string currentWord;
    std::string wordInProgress;
    std::vector<char> guessedLetters;
    std::vector<Player> players;
    int roundDuration;
    int currentRound;
    int roundsAmount;
    int difficulty;
    bool isGameActive;
    std::chrono::steady_clock::time_point timeStart;

    std::atomic<int> timeLeftInRound;
    std::atomic<bool> isRoundActive;
    std::thread timerThread;

    Game() {
        this->roundsAmount = 5;
        this->roundDuration = 60;
        this->currentRound = 0;
        this->difficulty = Levels::EASY;
        this->isGameActive = false;
        this->timeLeftInRound = 0;
        this->isRoundActive = false;
    }

    Game(int roundsAmount, int roundDuration, int difficulty) {
        this->roundsAmount = roundsAmount;
        this->roundDuration = roundDuration;
        this->currentRound = 0;
        this->difficulty = difficulty;
        this->isGameActive = false;
    }

    void initializeWordList();
    void startGame();
    void nextRound();
    void encodeWord();
    void startTimer();
    void stopTimer();
    void broadcastTimeToClients();
};

#endif // GAME_H
