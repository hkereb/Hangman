#ifndef GAME_H
#define GAME_H

#include <string>
#include <sstream>
#include <iomanip>
#include <vector>
#include <chrono>
#include <algorithm>
#include <iostream>
#include <random>
#include <atomic>
#include <thread>

#include "Player.h"
#include "sendToClient.h"

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
    std::vector<Player*> players;
    int roundDuration;
    int currentRound;
    int roundsAmount;
    int difficulty;
    bool isGameActive;
    std::chrono::steady_clock::time_point timeStart;

    std::atomic<int> timeLeftInRound;
    std::atomic<bool> isRoundActive;
    std::thread timerThread;

    Game() :
    timeLeftInRound(0),
    isRoundActive(false) {}

    // move constructor
    Game(Game&& other) noexcept
        : wordList(std::move(other.wordList)),
          currentWord(std::move(other.currentWord)),
          wordInProgress(std::move(other.wordInProgress)),
          guessedLetters(std::move(other.guessedLetters)),
          players(std::move(other.players)),
          roundDuration(other.roundDuration),
          currentRound(other.currentRound),
          roundsAmount(other.roundsAmount),
          difficulty(other.difficulty),
          isGameActive(other.isGameActive),
          timeStart(other.timeStart),
          timeLeftInRound(other.timeLeftInRound.load()),
          isRoundActive(other.isRoundActive.load()) {

        if (other.timerThread.joinable()) {
            timerThread = std::move(other.timerThread);
        }
    }

    ~Game() {
        stopTimer(); 
    }

    void initializeWordList();
    void startGame();
    void nextRound();
    void endRound();
    void encodeWord();
    void startTimer();
    void stopTimer();
    void resetGame(int roundsAmount, int roundDuration, int difficulty);
    static std::string convertTime(int time);
};

#endif // GAME_H
