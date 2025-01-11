#include "Game.h"
#include "helpers.h"

void Game::initializeWordList() {
    std::ifstream file("../words.json");
    if (!file.is_open()) {
        std::cerr << "unable to open words.json" << std::endl;
        return;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string jsonContent = buffer.str();
    file.close();

    std::string difficultyLevel;
    switch (difficulty) {
        case Levels::EASY:
            difficultyLevel = "\"easy\"";
            break;
        case Levels::MEDIUM:
            difficultyLevel = "\"medium\"";
            break;
        case Levels::HARD:
            difficultyLevel = "\"hard\"";
            break;
        default:
            std::cerr << "wrong difficulty" << std::endl;
            return;
    }

    size_t startPos = jsonContent.find(difficultyLevel);
    startPos = jsonContent.find('[', startPos);
    size_t endPos = jsonContent.find(']', startPos);
    if (startPos == std::string::npos || endPos == std::string::npos) {
        std::cerr << "wrong difficulty level structure" << std::endl;
        return;
    }

    std::string wordsArray = jsonContent.substr(startPos + 1, endPos - startPos - 1);

    std::vector<std::string> availableWords;
    size_t pos = 0;
    while ((pos = wordsArray.find("\"word\":")) != std::string::npos) {
        size_t start = wordsArray.find('"', pos + 7);
        size_t end = wordsArray.find('"', start + 1);
        if (start == std::string::npos || end == std::string::npos) {
            break;
        }
        availableWords.push_back(wordsArray.substr(start + 1, end - start - 1));
        wordsArray = wordsArray.substr(end + 1);
    }

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

    for (const auto& player : players) {
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
    const int minutes = time/60;
    const int seconds = time%60;
    std::stringstream ss;
    ss << minutes << ":" << std::setw(2) << std::setfill('0') << seconds;

    return ss.str();
}

void Game::startTimer() {
    stopTimer();
    isRoundActive = true;
    timeLeftInRound = roundDuration;

    timerThread = std::thread([this]() {
        while (isRoundActive && timeLeftInRound > 0) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            timeLeftInRound--;

            std::string messageBody = convertTime(timeLeftInRound);
            for (const auto& player : players) {
                sendToClient(player->sockfd, "11", messageBody);
            }

            if (timeLeftInRound <= 0) {
                isRoundActive = false;

                // todo - być może zbędna wiadomość, bardziej liczy się zaczęcie nowej gry + reset timera
                // for (const auto& player : players) {
                //     sendToClient(player->sockfd, "12", "");
                // }
                sendToClient(players[0]->sockfd, "12", "");
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
    this->roundsAmount = roundsAmount;
    this->roundDuration = roundDuration;
    this->difficulty = difficulty;
    currentRound = 0;
    isGameActive = false;
    wordList.clear();
    currentWord.clear();
    wordInProgress.clear();
    guessedLetters.clear();

    stopTimer();

    for (auto& player : players) {
        player->points = 0;
    }
}

