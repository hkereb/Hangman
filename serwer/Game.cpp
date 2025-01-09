#include "Game.h"

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
    if (currentRound == roundsAmount) {
        isGameActive = false;  // zakończenie gry
        stopTimer();
        return;
    }

    currentRound++;  // zwiększenie numeru rundy

    // ustawienie słowa do odgadnięcia na aktualną rundę
    currentWord = wordList[currentRound - 1];
    std::cout << "new word to guess: " + currentWord + "\n";
    guessedLetters.clear();
    encodeWord();

    for (auto& player : players) {
        player->lives = player->maxLives;  // ustawianie domyślnej liczby żyć
    }

    stopTimer();
    startTimer();
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



std::string Game::convertTime(int time) {
    int minutes = time/60;
    int seconds = time%60;
    std::stringstream ss;
    ss << minutes << ":" << std::setw(2) << std::setfill('0') << seconds;

    return ss.str();
}

void Game::startTimer() {
    isRoundActive = true;
    timeLeftInRound = roundDuration;

    timerThread = std::thread([this]() {
        while (isRoundActive && timeLeftInRound > 0) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            timeLeftInRound--;

            std::string messageBody = convertTime(timeLeftInRound);
            for (const auto& player : players) {
                sendToClient(player.sockfd, "11", messageBody);
            }

            if (timeLeftInRound <= 0) {
                isRoundActive = false;

                for (const auto& player : players) {
                    sendToClient(player.sockfd, "12", "Round time over");
                }
            }
        }
    });
}

void Game::stopTimer() {
    isRoundActive = false;
    if (timerThread.joinable()) {
        timerThread.join();
    }
}

void Game::resetGame(int roundsAmount, int roundDuration, int difficulty) {
    roundsAmount = roundsAmount;
    roundDuration = roundDuration;
    difficulty = difficulty;
    currentRound = 0;
    isGameActive = false;
    wordList.clear();
    currentWord.clear();
    wordInProgress.clear();
    guessedLetters.clear();

    // Stop any running timer
    stopTimer();

    // Reset players' state if needed
    for (auto& player : players) {
        player.points = 0; // Reset scores or other player data
    }
}

