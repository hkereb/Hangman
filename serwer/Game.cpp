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
    // reset gry - potrzebny dla ponownego uruchomienia z tymi samymi ustawieniami
    isGameActive = true;
    currentRound = 0;
    wordInProgress = "";

    // inicjalizacja slow
    initializeWordList();

    // start pierwszej rundy
    nextRound();
}

void Game::nextRound() {
    if (currentRound >= roundsAmount) {
        isGameActive = false;  // zakonczenie gry
        return;
    }

    currentRound++;  // zwiekszenie numeru rundy

    // ustawienie slowa do odgadniecia na aktualna runde
    currentWord = wordList[currentRound - 1];
    std::cout << "new word to guess: " + currentWord + "\n";
    guessedLetters.clear();
    encodeWord();

    for (auto& player : players) {
        player.lives = player.maxLives;  // ustawianie domyslnej liczby żyć
    }
}

void Game::encodeWord() {
    wordInProgress.clear();
    for (char c : currentWord) {
        if (std::find(guessedLetters.begin(), guessedLetters.end(), c) != guessedLetters.end()) {
            wordInProgress += c;
        } else {
            wordInProgress += '_';
        }
    }
}
