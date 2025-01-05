#include "Game.h"
#include <iostream>
#include <random>

void Game::initializeWordList() {
    std::vector<std::string> availableWords = {
        "coddles","saccharinity","windmilling","negativism","contingents",
        "ultradry","ecdyses","campong","demoiselle","mahjongs","fathomable",
        "mescluns","wristlets","polymorphically","supremacists","forzandi",
        "trilobal","crosstrees","plopped","gainsayer","transmissive","unthink",
        "taximan","cations","preclearances","pretorial","interdominion","bregmate",
        "osiers","fighter","gaudier","crowberries","archrivals","roamers","whiffets",
        "aciculas","exhedrae","florilegia","catalo","reconnection","fillip","searched"
    };

    std::random_device rd;
    std::default_random_engine engine(rd());
    std::shuffle(availableWords.begin(), availableWords.end(), engine);
    wordList.assign(availableWords.begin(), availableWords.begin() + roundsAmount);
}

void Game::startGame() {
    isGameActive = true;
    currentRound = 0;
    wordInProgress = "";
    initializeWordList();
    nextRound();
}

void Game::nextRound() {
    if (currentRound >= roundsAmount) {
        isGameActive = false;
        return;
    }

    currentRound++;
    currentWord = wordList[currentRound - 1];
    encodeWord(currentWord, guessedLetters);

    for (auto& player : players) {
        player.lives = player.maxLives;
    }
}

void Game::encodeWord(const std::string& currentWord, std::vector<char> guessedLetters) {
    wordInProgress.clear();
    for (char c : currentWord) {
        if (std::find(guessedLetters.begin(), guessedLetters.end(), c) != guessedLetters.end()) {
            wordInProgress += c;
        } else {
            wordInProgress += '_';
        }
    }
}
