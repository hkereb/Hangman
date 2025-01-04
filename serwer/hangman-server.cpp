#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <sys/epoll.h>

#include <iostream>
#include <list>
#include <string>
#include <utility>
#include <vector>
#include <algorithm>
#include <chrono>
#include <sstream>
#include <map>
#include <random>
#include <memory>
#include <set>

#define MAXEPOLLSIZE 1000
#define BACKLOG 200 // how many pending connections queue will hold
#define MAXPLAYERS 5

namespace Levels {
    static const int EASY = 1;
    static const int MEDIUM = 2;
    static const int HARD = 3;
}

//std::map<std::string, int> parseSettings(const std::string& settings);
void sendToClient(int clientFd, const std::string& commandNumber, const std::string& body);
void handleClientMessage(int clientFd, std::string msg);
int setNonBlocking(int sockfd);
int startListening();

struct Settings {
    std::string name;
    std::string password;
    int difficulty;
    int roundsAmount;
    int roundDurationSec;
};

struct Player {
    int sockfd;
    std::string nick;
    std::string lobbyName;

    bool isOwner = false;

    int points;
    int lives;
    int maxLives;
    std::vector<char> guessedLetters;
    std::vector<char> failedLetters;

    Player() {
        this->nick = "";
        this->sockfd = NULL;
        this->isOwner = false;
        this->points = 0;
        this->maxLives = 5;
        this->lives = this->maxLives;
        this->lobbyName = "";
        this->failedLetters = {};
    }
};

struct Game {
    std::vector<std::string> wordList;
    std::string currentWord;
    std::vector<char> guessedLetters;
    std::vector<Player> players;
    int roundDuration;
    int currentRound;
    int roundsAmount;
    int difficulty;
    bool isGameActive;
    std::chrono::steady_clock::time_point timeStart; // czas rozpoczęcia gry

    Game() {
        this->roundsAmount = 5;
        this->roundDuration = 60;
        this->currentRound = 0;
        this->difficulty = Levels::EASY;
        this->isGameActive = false;
    }

    Game(int roundsAmount, int roundDuration, int difficulty) {
        this->roundsAmount = roundsAmount;
        this->roundDuration = roundDuration;
        this->currentRound = 0;
        this->difficulty = difficulty;
        this->isGameActive = false;
    }

    void initializeWordList() {
        std::vector<std::string> availableWords = { // poczatkowa lista wzieta z losowego api
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
        std::shuffle(availableWords.begin(), availableWords.end(), engine); // shuffle zeby byla jakas losowosc
        wordList.assign(availableWords.begin(), availableWords.begin() + roundsAmount);
    }

    void startGame() {
        // reset gry - potrzebny dla ponownego uruchomienia z tymi samymi ustawieniami
        isGameActive = true;
        currentRound = 0;
        guessedLetters.clear();

        // inicjalizacja slow
        initializeWordList();

        // start pierwszej rundy
        nextRound();
    }

    void nextRound() {
        if (currentRound >= roundsAmount) {
            isGameActive = false;  // zakonczenie gry
            return;
        }

        currentRound++;  // zwiekszenie numeru rundy

        // ustawienie slowa do odgadniecia na aktualna runde
        currentWord = wordList[currentRound - 1];
        guessedLetters.resize(currentWord.size(), '_');  // resetowanie odgadnietych liter

        for (auto& player : players) {
            player.lives = player.maxLives;  // ustawianie domyslnej liczby żyć
        }
    }
};

struct Lobby {
    std::string name;
    std::string password;
    int playersCount;
    std::vector<Player> players;
    Player owner;
    std::map<int, time_t> joinTimes;

    int difficulty;
    int roundsAmount;
    int roundDuration;

    Game game;

    Lobby() {
        this->name = "";
        this->password = "";
        this->playersCount = 0;
        this->difficulty = 1;
        this->roundsAmount = 5;
        this->roundDuration = 60;
    }

    void startGame() {
        for (auto& player : players) {  // resetowanie punktow przed rozpoczeciem gry
            player.points = 0;
        }

        if (players.size() < 2) { // w lobby musi byc minimalnie 2 graczy
            std::cout << "Potrzeba przynajmniej 2 graczy w lobby\n";
            return;
        }

        game = Game(roundsAmount, roundDuration, difficulty);
        game.players = players;
        game.startGame();

        std::cout << "Gra rozpoczęta w lobby: " << name << "\n";
    }

    // TODO przetestować (musi być obsługa usuwania danych graczy)
    void setOwner() {
        // if (!players.empty()) { // jeżeli lobby nie ma graczy to jest usuwane
        //     auto owner = std::min_element(players.begin(), players.end(),
        //         [this](const Player& a, const Player& b) {
        //             return joinTimes[a.sockfd] < joinTimes[b.sockfd];
        //         });
        //     for (auto& player : players) {
        //         player.isOwner = (&player == &(*owner));
        //     }
        //
        // }
        owner = players[0];
        players[0].isOwner = true;
    }
};

std::vector<Player> players;
std::vector<std::string> playersNicknames;

std::list<Lobby> gameLobbies;
int lobbyCount = 0;
std::vector<std::string> lobbyNames;

void sendToClient(int clientFd, const std::string& commandNumber, const std::string& body) {
    if (commandNumber.size() != 2) {
        std::cerr << "Error: Command number must consist of 2 characters";
        return;
    }

    std::string fullMessage = commandNumber + "\\" + body + "\n";
    ssize_t bytesSent = send(clientFd, fullMessage.c_str(), fullMessage.size(), 0);

    if (bytesSent == -1) {
        std::cerr << "Error: Failed to send message to client with socket " << clientFd << ".\n";
    }
}

std::string messageSubstring(std::string msg) {
    return msg.substr(3);
}

void sendLobbiesToClients(std::vector<std::string> lobbyNames, int clientFd = -1) {
    std::string messageBody;
    // przygotowanie wiadomości
    for (size_t i = 0; i < lobbyNames.size(); ++i) {
        messageBody += lobbyNames[i];
        if (i != lobbyNames.size() - 1) {
            messageBody += ",";
        }
    }
    // wysłanie do jednego klienta (który dopiero włączył aplikację)
    if (clientFd != -1) {
        sendToClient(clientFd, "70", messageBody);
    }
    // wysłanie do wielu klientów (update dla klientów, którzy są już w aplikacji)
    else {
        for (const auto& player : players) {
            if (player.lobbyName.empty()) {
                sendToClient(player.sockfd, "70", messageBody);
            }
        }
    }
}

void sendPlayersToClients(const Lobby* lobby) {
    std::string msgBody;

    // przygotowanie wiadomości
    for (size_t i = 0; i < lobby->players.size(); ++i) {
        msgBody += lobby->players[i].nick;
        if (i != lobby->players.size() - 1) {
            msgBody += ",";
        }
    }

    // wysłanie do wszystkich klientów z pokoju
    for (const auto& player : lobby->players) {
        sendToClient(player.sockfd, "71", msgBody);
    }
}

void isStartAllowed(const Lobby* lobby) {
    if (!lobby->game.isGameActive) {
        if (lobby->playersCount >= 2) {
            sendToClient(lobby->players[0].sockfd, "72", "1");
        }
        else {
            sendToClient(lobby->players[0].sockfd, "72", "0");
        }
    }
}

/*
Lobby* findPlayerLobby(int client_fd, bool master = false) {
    for (auto& lobby : gameLobbies) {
        if (master) {
            if (!lobby.players.empty() && lobby.players.front().sockfd == client_fd) {
                return &lobby;
            }
        }
        else {
            auto it = std::find_if(lobby.players.begin(), lobby.players.end(), [client_fd](const Player& player) {
                return player.sockfd == client_fd;
            });
            if (it != lobby.players.end()) {
                return &lobby;
            }
        }
    }
    return nullptr;
}
*/

/*
std::string encodeWord(std::string word, std::set<char> guessed_letters) {
    std::string encoded_word;

    for (char c : word) {
        if (guessed_letters.count(c) > 0) {
            encoded_word += c;
        } else {
            encoded_word += '_';
        }
    }
    return encoded_word;
}
*/

/*
void startTheGame(Lobby* lobby) {
    lobby->current_word_full = lobby->word_que[lobby->word_iterator];
    lobby->is_active = true;
    std::string secret_word = encodeWord(lobby->current_word_full, lobby->all_guessed_letters);

    for (const auto& player : lobby->players) {
        sendToClient(player.sockfd, "08", secret_word);
    }
}*/

Settings parseSettings(std::string msg) {
    Settings settings;

    size_t posName = msg.find("name:");
    size_t posPass = msg.find("password:");
    size_t posDiff = msg.find("difficulty:");
    size_t posRounds = msg.find("rounds:");
    size_t posTime = msg.find("time:");

    size_t nameStart = posName + strlen("name:"); // po "name:"
    size_t nameEnd = msg.find(",", nameStart); // do przecinka
    settings.name = msg.substr(nameStart, nameEnd - nameStart);

    size_t passStart = posPass + strlen("password:");
    size_t passEnd = msg.find(",", passStart);
    settings.password = msg.substr(passStart, passEnd - passStart);

    size_t diffStart = posDiff + strlen("difficulty:");
    size_t diffEnd = msg.find(",", diffStart);
    settings.difficulty = std::stoi(msg.substr(diffStart, diffEnd - diffStart));

    size_t roundsStart = posRounds + strlen("rounds:");
    size_t roundsEnd = msg.find(",", roundsStart);
    settings.roundsAmount = std::stoi(msg.substr(roundsStart, roundsEnd - roundsStart));

    size_t timeStart = posTime + strlen("time:");
    size_t timeEnd = msg.find(",", timeStart);
    settings.roundDurationSec = std::stoi(msg.substr(timeStart, timeEnd - timeStart));

    return settings;
}

// obsługa klienta na podstawie jego wiadomości
void handleClientMessage(int clientFd, std::string msg) {
    if (msg.substr(0, 2) == "01") {// Ustawienie nicku
        std::string nick = messageSubstring(msg);

        if (std::find(playersNicknames.begin(), playersNicknames.end(), nick) != playersNicknames.end()) {
            std::cout << "Nick, " << nick << ", has already been taken.\n";
            // powiadomienie strony klienta o niepowodzeniu
            sendToClient(clientFd, "01", "0");
            return;
        }

        auto playerIt = std::find_if(players.begin(), players.end(), [clientFd](const Player& player) {
            return player.sockfd == clientFd;
        });

        if (playerIt != players.end()) {
            playerIt->nick = nick;
            playersNicknames.push_back(nick);
            sendToClient(clientFd, "01", "1");
            //std::cout << "Player's accepted nickname: " << playerIt->nick;
        } else {
            std::cout << "Player's socket has not been found in the players vector (socket: " << clientFd << ")";
            sendToClient(clientFd, "01", "0");
            return;
        }
    } else if (msg.substr(0, 2) == "02") {  // Tworzenie pokoju
        std::string settings = messageSubstring(msg);
        Settings parsedSettings = parseSettings(settings);

        std::string lobbyName = parsedSettings.name;

        // sprawdzenie czy nazwa nie jest zajęta
        auto lobbyIt = std::find_if(gameLobbies.begin(), gameLobbies.end(), [&lobbyName](const Lobby& lobby) {
            return lobby.name == lobbyName;
        });

        if (lobbyIt != gameLobbies.end()) {
            // TODO w przypadku błędu trzeba dokładnie podać gdzie on wystąpił (np. 02\11011, gdzie 0 oznacza błąd w trzeciej opcji)
            sendToClient(clientFd, "02", "0");  // Send failure response
            std::cout << "Failed to create lobby. Name already taken: " << lobbyName << "\n";
            return;
        }

        // nowe lobby
        Lobby newLobby;
        newLobby.name = lobbyName;
        newLobby.password = parsedSettings.password;  // todo: password handling
        newLobby.difficulty = parsedSettings.difficulty;
        newLobby.roundsAmount = parsedSettings.roundsAmount;
        newLobby.roundDuration = parsedSettings.roundDurationSec;

        // lokalizowanie gracza tworzacego lobby
        auto playerIt = std::find_if(players.begin(), players.end(), [clientFd](const Player& player) {
            return player.sockfd == clientFd;
        });

        if (playerIt != players.end()) {
            // Add the player to the lobby
            newLobby.players.push_back(*playerIt);
            newLobby.playersCount++;
            newLobby.setOwner();
        }

        // dodanie lobby do listy
        gameLobbies.push_back(newLobby);
        lobbyCount++;
        lobbyNames.push_back(lobbyName);

        sendLobbiesToClients(lobbyNames);
        sendPlayersToClients(&newLobby);

        sendToClient(clientFd, "02", "1");  // stworzono lobby
        std::cout << "Lobby created successfully: " << lobbyName << "\n";

        std::cout << "Current lobby count: " << lobbyCount << "\n";
    }
    else if (msg.substr(0, 2) == "03") {  // Dołączanie do pokoju
        std::string lobbyInfo = messageSubstring(msg);

        size_t posName = msg.find("name:");
        size_t posPass = msg.find("password:");

        size_t nameStart = posName + strlen("name:"); // po "name:"
        size_t nameEnd = msg.find(",", nameStart); // do przecinka
        std::string lobbyName = msg.substr(nameStart, nameEnd - nameStart);

        size_t passStart = posPass + strlen("password:");
        size_t passEnd = msg.find(",", passStart);
        std::string password = msg.substr(passStart, passEnd - passStart);

        std::cout << "taką widzę nazwę: " + lobbyName + "\n";
        std::cout << "takie widzę hasło: " + password + "\n";

        // Znajdź pokój
        auto lobbyIt = std::find_if(gameLobbies.begin(), gameLobbies.end(), [&lobbyName](const Lobby& lobby) {
            return lobby.name == lobbyName;
        });

        if (password != lobbyIt->password) {
            sendToClient(clientFd, "03", "0");  // niepoprawne hasło
            return;
        }

        // TODO należy zwracać konkretniejsze kody błędów
        if (lobbyIt == gameLobbies.end()) {
            sendToClient(clientFd, "03", "0");  // Pokój nie istnieje
            return;
        }

        // powiadomienie ze pokoj jest pelny
        if (lobbyIt->playersCount >= MAXPLAYERS) {
            sendToClient(clientFd, "03", "0");
            return;
        }

        // wyszukanie gracza ktory ma dolaczyc do pokoju
        auto playerIt = std::find_if(players.begin(), players.end(), [clientFd](const Player& player) {
            return player.sockfd == clientFd;
        });
        
        if (playerIt != players.end()) {
            time_t now = time(nullptr);
            lobbyIt->joinTimes[clientFd] = now;
            lobbyIt->players.push_back(*playerIt);  // Dodaj gracza do pokoju
            lobbyIt->playersCount++;
            sendToClient(clientFd, "03", "1");  // Sukces
            // TODO fix
            auto& lobby = *lobbyIt;
            sendPlayersToClients(&lobby);
            isStartAllowed(&lobby);
        } else {
            sendToClient(clientFd, "03", "0");  // Nie znaleziono gracza
        }
    }
    else if (msg.substr(0, 2) == "04") {  // Ustawienia pokoju
        std::string settings = messageSubstring(msg);

        // parsowanie ustawien
        auto parsedSettings = parseSettings(settings);

        // Find the lobby for the client
        auto lobbyIt = std::find_if(gameLobbies.begin(), gameLobbies.end(), [clientFd](const Lobby& lobby) {
            return std::any_of(lobby.players.begin(), lobby.players.end(), [clientFd](const Player& player) {
                return player.sockfd == clientFd;
            });
        });

        if (lobbyIt != gameLobbies.end()) {
            auto ownerIt = std::find_if(lobbyIt->players.begin(), lobbyIt->players.end(), [](const Player& player) {
                return player.isOwner;  // szukamy obecnego wlasciciela lobby
            });

            // if (ownerIt != lobbyIt->players.end() && ownerIt->sockfd == clientFd) {
            //     // przypisanie ustawien
            //     if (parsedSettings.find("difficulty") != parsedSettings.end()) {
            //         lobbyIt->difficulty = parsedSettings["difficulty"];
            //     }
            //     if (parsedSettings.find("roundsAmount") != parsedSettings.end()) {
            //         lobbyIt->roundsAmount = parsedSettings["roundsAmount"];
            //     }
            //     if (parsedSettings.find("roundDuration") != parsedSettings.end()) {
            //         lobbyIt->roundDuration = parsedSettings["roundDuration"];
            //     }
            //
            //     // wypisanie ustawien
            //     std::cout << "Updated settings for lobby " << lobbyIt->name << ":\n";
            //     std::cout << "  Difficulty: " << lobbyIt->difficulty << "\n";
            //     std::cout << "  Rounds: " << lobbyIt->roundsAmount << "\n";
            //     std::cout << "  Duration: " << lobbyIt->roundDuration << "\n";
            //     sendToClient(clientFd, "04", "1\nsettingsAssignmentSuccessful\n");  // Success
            // } else {
            //     sendToClient(clientFd, "04", "0\nnotLobbyLeader\n");  // Gracz nie jest liderem
            // }
        } else {
            sendToClient(clientFd, "04", "0\nsettingsAssignmentFailed\n");  // Failed
        }
    }
    // TODO wiele rzeczy jest tu ogarnianych przez gui, zmodyfikować
    else if (msg.substr(0, 2) == "05") {  // Start gry
        auto lobbyIt = std::find_if(gameLobbies.begin(), gameLobbies.end(), [clientFd](const Lobby& lobby) {
            return std::any_of(lobby.players.begin(), lobby.players.end(), [clientFd](const Player& player) {
                return player.sockfd == clientFd;
            });
        });

        if (lobbyIt != gameLobbies.end()) {
            auto ownerIt = std::find_if(lobbyIt->players.begin(), lobbyIt->players.end(), [](const Player& player) {
                return player.isOwner;  // szukamy obecnego wlasciciela lobby
            });

            if (ownerIt != lobbyIt->players.end() && ownerIt->sockfd == clientFd) {
                // Rozpoczęcie gry w tym lobby
                if (lobbyIt->players.size() < 2) {
                    sendToClient(clientFd, "05", "0\nnotEnoughPlayers\n");
                } else {
                    lobbyIt->startGame();
                    for (const auto& player : lobbyIt->players) {  // powiadomienie graczy
                        sendToClient(player.sockfd, "05", "1\ngameStarted\n");
                    }
                }
            } else {
                sendToClient(clientFd, "05", "0\nnotLobbyLeader\n");  // Gracz nie jest liderem
            }
        } else {
            sendToClient(clientFd, "05", "0\ngameStartFailed\n");  // Pokój nie istnieje
        }
    } else if (msg.substr(0, 2) == "06") {  // Próba zgadnięcia litery
        char letter = msg[2];

        auto lobbyIt = std::find_if(gameLobbies.begin(), gameLobbies.end(), [clientFd](const Lobby& lobby) {
            return std::any_of(lobby.players.begin(), lobby.players.end(), [clientFd](const Player& player) {
                return player.sockfd == clientFd;
            });
        });

        if (lobbyIt != gameLobbies.end()) {
            auto& game = lobbyIt->game;
            auto playerIt = std::find_if(lobbyIt->players.begin(), lobbyIt->players.end(), [clientFd](const Player& player) {
                return player.sockfd == clientFd;
            });

            if (playerIt != lobbyIt->players.end()) {
                if (playerIt->lives <= 0) {
                    sendToClient(clientFd, "06", "0\nnoLivesLeft\n");
                    return;
                }

                if (std::find(game.guessedLetters.begin(), game.guessedLetters.end(), letter) != game.guessedLetters.end()) {
                    sendToClient(clientFd, "06", "0\nletterAlreadyGuessed\n");
                    return;
                }

                bool isCorrect = false;
                for (size_t i = 0; i < game.currentWord.size(); ++i) {
                    if (game.currentWord[i] == letter && game.guessedLetters[i] == '_') {
                        game.guessedLetters[i] = letter;
                        isCorrect = true;
                    }
                }

                if (isCorrect) {
                    int occurrences = std::count(game.currentWord.begin(), game.currentWord.end(), letter);
                    playerIt->points += 25 * occurrences;

                        std::string currentWordState(game.guessedLetters.begin(), game.guessedLetters.end());
                    for (const auto& player : lobbyIt->players) {
                        sendToClient(player.sockfd, "06", "1\nwordState\n" + currentWordState + "\n");
                    }

                    if (std::all_of(game.guessedLetters.begin(), game.guessedLetters.end(), [](char c) { return c != '_'; })) {
                        for (const auto& player : lobbyIt->players) {
                            sendToClient(player.sockfd, "06", "1\nwordGuessed\n" + game.currentWord + "\n");
                        }
                        game.nextRound();
                    }
                } else {
                    playerIt->lives--;
                    if (playerIt->lives > 0) {
                        sendToClient(clientFd, "06", "0\nwrongGuess\n");
                    } else {
                        sendToClient(clientFd, "06", "0\nnoLivesLeft\n");

                        if (std::all_of(lobbyIt->players.begin(), lobbyIt->players.end(), [](const Player& player) { return player.lives <= 0; })) {
                            for (const auto& player : lobbyIt->players) {
                                sendToClient(player.sockfd, "06", "0\nallPlayersOut\n");
                            }
                            game.nextRound();
                        }
                    }
                }
            } else {
                sendToClient(clientFd, "06", "0\nplayerNotFound\n");
            }
        } else {
            sendToClient(clientFd, "06", "0\nlobbyNotFound\n");
        }
    } else if (msg.substr(0, 2) == "07") {  // Restart gry
        std::string roomName = messageSubstring(msg);

        auto lobbyIt = std::find_if(gameLobbies.begin(), gameLobbies.end(), [&roomName](const Lobby& lobby) {
            return lobby.name == roomName;
        });


        if (lobbyIt != gameLobbies.end()) {
            auto ownerIt = std::find_if(lobbyIt->players.begin(), lobbyIt->players.end(), [](const Player& player) {
                return player.isOwner;  // szukamy obecnego wlasciciela lobby
            });

            if (ownerIt != lobbyIt->players.end() && ownerIt->sockfd == clientFd) {
                std::cout << "Restart gry w pokoju: " << roomName << "\n";
                // restart gry w tym lobby
                lobbyIt->startGame();
                for (const auto& player : lobbyIt->players) {  // powiadomienie graczy
                    sendToClient(player.sockfd, "07", "1\ngameRestartSuccessful\n");
                }
            } else {
                sendToClient(clientFd, "07", "0\nnotLobbyLeader\n");  // Gracz nie jest liderem
            }
        } else {
            sendToClient(clientFd, "07", "0\ngameRestartFailed\n");  // Pokój nie istnieje
        }
    } else if (msg.substr(0, 2) == "08") {  // Stan gry
        std::string roomName = messageSubstring(msg);


        auto lobbyIt = std::find_if(gameLobbies.begin(), gameLobbies.end(), [&roomName](const Lobby& lobby) {
            return lobby.name == roomName;
        });

        if (lobbyIt != gameLobbies.end()) {
            std::string gameState = "Game state in room: " + roomName + "\n";
            gameState += "Current round: " + std::to_string(lobbyIt->game.currentRound) + "/" + std::to_string(lobbyIt->roundsAmount) + "\n";
            gameState += "Game over: " + std::string(lobbyIt->game.isGameActive ? "Yes" : "No") + "\n";
            gameState += "Difficulty: " + std::to_string(lobbyIt->difficulty) + "\n";

            for (auto& player : lobbyIt->players) {
                gameState += player.nick + ": " + std::to_string(player.points) + "\n";
            }

            gameState += "Guessing word: " + lobbyIt->game.currentWord + "\n";
            gameState += "Guessed letters: ";
            for (char letter : lobbyIt->game.guessedLetters) {
                gameState += letter;
            }

            sendToClient(clientFd, "08", gameState);  // Stan gry
        } else {
            sendToClient(clientFd, "08", "0\ngameStateReadFailed\n");
        }
    } else if (msg.substr(0, 2) == "09") {  // Opuszczenie pokoju
        std::string roomName = messageSubstring(msg);

        auto lobbyIt = std::find_if(gameLobbies.begin(), gameLobbies.end(), [&roomName](const Lobby& lobby) {
            return lobby.name == roomName;
        });

        if (lobbyIt != gameLobbies.end()) {
            auto playerIt = std::find_if(lobbyIt->players.begin(), lobbyIt->players.end(), [clientFd](const Player& player) {
                return player.sockfd == clientFd;
            });

            if (playerIt != lobbyIt->players.end()) {
                if (playerIt->isOwner) {  // jezeli byl to gracz przestaje byc ownerem
                    playerIt->isOwner = false;
                }

                lobbyIt->joinTimes.erase(clientFd);  // usuwanie gracza z listy czasow dolaczania

                lobbyIt->players.erase(playerIt);  // usuwanie gracza z listy graczy
                lobbyIt->playersCount--;

                if (!lobbyIt->players.empty()) {
                    lobbyIt->setOwner();  // ustawienie nowego ownera gdyby obecny wlasnie wyszedl
                }

            }

            sendToClient(clientFd, "09", "1\nplayerLeftRoomSuccessfully\n");  // Sukces
        } else {
            sendToClient(clientFd, "09", "0\nplayerLeftRoomFailed\n");
        }
    }
    // TODO zmienić numer komendy
    else if (msg.substr(0, 2) == "10") {  // Informacje o lobby
        auto lobbyIt = std::find_if(gameLobbies.begin(), gameLobbies.end(), [clientFd](const Lobby& lobby) {
            return std::any_of(lobby.players.begin(), lobby.players.end(), [clientFd](const Player& player) {
                return player.sockfd == clientFd;
            });
        });

        if (lobbyIt != gameLobbies.end()) {
            auto ownerIt = std::find_if(lobbyIt->players.begin(), lobbyIt->players.end(),
                [](const Player& player) { return player.isOwner; });
            std::string ownerNick;

            if (ownerIt != lobbyIt->players.end()) {
                ownerNick = ownerIt->nick;
            } else {
                ownerNick = "None";
            }
            std::string response = "10\nroomFound\n" + lobbyIt->name +
                                "\nPlayers: " + std::to_string(lobbyIt->playersCount) +
                                "\nOwner: " + ownerNick + "\n";
            sendToClient(clientFd, "10", response);
        } else {
            sendToClient(clientFd, "10", "0\ngettingResponseFailed\n");
        }
    }
    else if (msg.substr(0, 2) == "70") { // prośba o listę pokoi (dla jednego gracza)
        sendLobbiesToClients(lobbyNames, clientFd);
    }
    /*
    else if (msg.substr(0, 2) == "12") {
        Lobby* lobby = findPlayerLobby(client_fd, true);
        startTheGame(lobby);
    }
     */
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[]) {
    if (argc != 1) {
        fprintf(stderr, "no extra arguments required\n");
        return 1;
    }

    int sockfd = startListening();

    int efd = epoll_create(MAXEPOLLSIZE);

    epoll_event ev{};
    ev.events = EPOLLIN;
    ev.data.fd = sockfd;

    if (epoll_ctl(efd, EPOLL_CTL_ADD, sockfd, &ev) < 0) {
        perror("epoll_ctl");
        return -1;
    } else {
        printf("success insert listening socket into epoll.\n");
    }

    int fdsToWatch = 1;
    struct epoll_event *events;
    events = (struct epoll_event *)malloc(sizeof(struct epoll_event) * fdsToWatch);

    while (1) {  // loop for accepting incoming connections
        int ready = epoll_wait(efd, events, fdsToWatch, -1);
        if (ready == -1) {
            perror("epoll_wait");
            break;
        }

        for (int n = 0; n < ready; ++n) {
            // TODO: usuwanie danych klienta ze wszystkich struktur po zerwaniu połączenia
            if (events[n].data.fd == sockfd) {
                // std::cout << "sockfd" << std::endl;
                sockaddr_in clientAddr{};
                socklen_t addrSize = sizeof(clientAddr);
                int newFd = accept(events[n].data.fd, (struct sockaddr *)&clientAddr, &addrSize);
                if (newFd == -1) {
                    if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                        break;
                    } else {
                        perror("accept");
                        break;
                    }
                }

                printf("server: new connection established.\n");
                setNonBlocking(newFd);
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = newFd;
                if (epoll_ctl(efd, EPOLL_CTL_ADD, newFd, &ev) < 0) {
                    printf("Failed to insert socket into epoll.\n");
                }
                fdsToWatch++;

                Player newPlayer;
                newPlayer.sockfd = newFd;
                players.push_back(newPlayer);
            }
            else {
                //std::cout << "clientfd" << std::endl;
                while (true) {
                    char buffer[1024] = {0};
                    int bytesReceived = recv(events[n].data.fd, buffer, sizeof(buffer) - 1, 0);
                    if (bytesReceived < 0) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            // nie ma więcej danych
                            break;
                        }
                        // klient rozłączył się, należy usunąć jego dane
                        perror("recv (wiadomość od klienta)");
                        close(events[n].data.fd);
                        epoll_ctl(efd, EPOLL_CTL_DEL, events[n].data.fd, &ev);
                        fdsToWatch--;
                        break;
                    } else if (bytesReceived == 0) {
                        // klient się rozłączył
                        close(events[n].data.fd);
                        epoll_ctl(efd, EPOLL_CTL_DEL, events[n].data.fd, &ev);
                        fdsToWatch--;
                        break;
                    }
                    write(1, "klient: ", 8);
                    write(1, buffer, bytesReceived); // do sprawdzania odebranej wiadomości
                    write(1, "\n", 1);

                    std::string clientMessage(buffer, bytesReceived);

                    handleClientMessage(events[n].data.fd, clientMessage);
                }
            }
        }
    }

    free(events);
    close(sockfd);
    return 0;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////

// Funkcja ustawiająca gniazdo w tryb nieblokujący
int setNonBlocking(int sockfd) {
    int flags, s;
    flags = fcntl(sockfd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl");
        return -1;
    }
    flags |= O_NONBLOCK;
    s = fcntl(sockfd, F_SETFL, flags);
    if (s == -1) {
        perror("fcntl");
        return -1;
    }
    return 0;
}

int startListening() {
    int sockfd;

    addrinfo* resolved;
    addrinfo hints{};
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int status = getaddrinfo(NULL, "1111", &hints, &resolved);
    if (status != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        return 2;
    }

    struct addrinfo* addrIterator;
    // resolved now points to a linked list of 1 or more struct addrinfos
    for (addrIterator = resolved; addrIterator != nullptr; addrIterator = addrIterator->ai_next) {
        // make a socket:
        sockfd = socket(addrIterator->ai_family, addrIterator->ai_socktype, addrIterator->ai_protocol);
        if (sockfd == -1) {
            perror("socket");
            continue;
        }

        // make the sock non-blocking
        setNonBlocking(sockfd);

        // bind it to the port
        int failBind = bind(sockfd, addrIterator->ai_addr, addrIterator->ai_addrlen);
        if (failBind == -1) {
            close(sockfd);
            perror("bind");
            continue;
        }

        break;
    }

    if (addrIterator == nullptr) {
        fprintf(stderr, "failed to bind\n");
        return 2;
    }

    freeaddrinfo(resolved);  // free the linked-list

    // listen for incoming connection
    int failListen = listen(sockfd, BACKLOG);
    if (failListen == -1) {
        perror("listen");
        exit(1);
    }

    printf("waiting for connections on port 1111...\n");

    return sockfd;
}