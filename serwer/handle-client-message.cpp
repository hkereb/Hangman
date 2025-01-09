#include "handle-client-message.h"
// Assuming external variables or definitions are declared elsewhere
extern std::vector<Player> players;
extern std::vector<Lobby> lobbies;
extern std::vector<std::string> playersNicknames;
extern std::vector<std::string> lobbyNames;
extern int lobbyCount;


// obsługa klienta na podstawie jego wiadomości
void handleClientMessage(int clientFd, std::string msg) {
    auto playerIt = std::find_if(players.begin(), players.end(), [clientFd](const Player& player) {
            return player.sockfd == clientFd;
        });

    if (msg.substr(0, 2) == "01") {// Ustawienie nicku
        std::string nick = messageSubstring(msg);

        if (std::find(playersNicknames.begin(), playersNicknames.end(), nick) != playersNicknames.end()) {
            std::cout << "Nick, " << nick << ", has already been taken.\n";
            // powiadomienie strony klienta o niepowodzeniu
            sendToClient(clientFd, "01", "0");
            return;
        }

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
    }
    else if (msg.substr(0, 2) == "02") {  // Tworzenie pokoju
        std::string settings = messageSubstring(msg);
        Settings parsedSettings = parseSettings(settings);

        std::string lobbyName = parsedSettings.name;

        // sprawdzenie czy nazwa nie jest zajęta
        auto lobbyIt = std::find_if(lobbies.begin(), lobbies.end(), [&lobbyName](const Lobby& lobby) {
            return lobby.name == lobbyName;
        });

        if (lobbyIt != lobbies.end()) {
            // TODO w przypadku błędu trzeba dokładnie podać gdzie on wystąpił (np. 02\11011, gdzie 0 oznacza błąd w trzeciej opcji, tak najłatwiej będzie dekodować błąd)
            sendToClient(clientFd, "02", "0");  // Send failure response
            std::cout << "Failed to create lobby. Name already taken: " << lobbyName << "\n";
            return;
        }

        // nowe lobby
        Lobby newLobby;
        newLobby.name = lobbyName;
        newLobby.password = parsedSettings.password;
        newLobby.difficulty = parsedSettings.difficulty;
        newLobby.roundsAmount = parsedSettings.roundsAmount;
        newLobby.roundDuration = parsedSettings.roundDurationSec;

        if (playerIt != players.end()) {
            // Add the player to the lobby
            newLobby.players.push_back(*playerIt);
            newLobby.playersCount++;
            newLobby.setOwner();
        }

        // dodanie lobby do listy
        lobbies.push_back(newLobby);
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

        // Znajdź pokój
        auto lobbyIt = std::find_if(lobbies.begin(), lobbies.end(), [&lobbyName](const Lobby& lobby) {
            return lobby.name == lobbyName;
        });

        if (password != lobbyIt->password) {
            sendToClient(clientFd, "03", "0");  // niepoprawne hasło
            return;
        }

        // TODO należy zwracać konkretniejsze kody błędów (nie tylko tu)
        if (lobbyIt == lobbies.end()) {
            sendToClient(clientFd, "03", "0");  // Pokój nie istnieje
            return;
        }

        // powiadomienie ze pokoj jest pelny
        if (lobbyIt->playersCount >= MAXPLAYERS) {
            sendToClient(clientFd, "03", "0");
            return;
        }

        if (playerIt != players.end()) {
            time_t now = time(nullptr);
            lobbyIt->joinTimes[clientFd] = now;
            lobbyIt->players.push_back(*playerIt);  // Dodaj gracza do pokoju
            lobbyIt->playersCount++;
            sendToClient(clientFd, "03", "1");  // Sukces
            auto& lobby = *lobbyIt;
            sendPlayersToClients(&lobby);
            isStartAllowed(&lobby);
        }
        else {
            sendToClient(clientFd, "03", "0");  // Nie znaleziono gracza
        }
    }
    else if (msg.substr(0, 2) == "73") {  // Start gry
        // todo tutaj szukać tylko po ownerach (players[0])
        auto lobbyIt = std::find_if(lobbies.begin(), lobbies.end(), [clientFd](const Lobby& lobby) {
            return std::any_of(lobby.players.begin(), lobby.players.end(), [clientFd](const Player& player) {
                return player.sockfd == clientFd;
            });
        });

        if (lobbyIt != lobbies.end()) {
            lobbyIt->startGame();
            for (const auto& player : lobbyIt->players) {  // powiadomienie graczy
                sendStartToClients(&*lobbyIt);
            }
        }
        else {
            sendToClient(clientFd, "73", "0");  // Pokój nie istnieje
        }
    }
    else if (msg.substr(0, 2) == "06") {  // Próba zgadnięcia litery
       char letter = messageSubstring(msg)[0];

        auto lobbyIt = std::find_if(lobbies.begin(), lobbies.end(), [clientFd](const Lobby& lobby) {
            return std::any_of(lobby.players.begin(), lobby.players.end(), [clientFd](const Player& player) {
                return player.sockfd == clientFd;
            });
        });

        if (lobbyIt == lobbies.end()) {
            sendToClient(clientFd, "06", "0"); // nie znaleziono lobby
            return;
        }
        auto& game = lobbyIt->game;

        if (playerIt == lobbyIt->players.end()) {
            sendToClient(clientFd, "06", "0"); // nie znaleziono gracza
        }

        if (playerIt->lives == 0) {
            sendToClient(clientFd, "06", "0"); // brak żyć TO POPRAWIĆ
            return;
        }

        // todo sprawdzić czy litera jest w failed klienta

        if (!game.guessedLetters.empty()) {
            if (std::find(game.guessedLetters.begin(), game.guessedLetters.end(), letter) != game.guessedLetters.end()) {
                sendToClient(clientFd, "06", "2," + letter); // litera już zgadnięta
                return;
            }
        }
        if (!playerIt->failedLetters.empty()) {
            if (std::find(playerIt->failedLetters.begin(), playerIt->failedLetters.end(), letter) != playerIt->failedLetters.end()) {
                sendToClient(clientFd, "06", "3," + letter); // litera została już wcześniej wykorzystana jako fail
                return;
            }
        }

        bool isCorrect = false;
        for (size_t i = 0; i < game.currentWord.size(); ++i) {
            if (game.currentWord[i] == letter && game.wordInProgress[i] == '_') {
                game.wordInProgress[i] = letter;
                isCorrect = true;
            }
        }

        if (isCorrect) {
            // 1. update hasła w strukturze
            game.guessedLetters.push_back(letter);
            game.encodeWord();

            // 2. dodanie graczowi punktów
            int occurrences = std::count(game.currentWord.begin(), game.currentWord.end(), letter);
            playerIt->points += 25 * occurrences;

            // 3. wysłanie graczowi odpowiedzi na jego literę
            std::string msgBody = "1," + std::string(1,letter); //todo sprawdź czemu tu char nie działa
            sendToClient(clientFd, "06", msgBody);

            // 4. powiadomienie wszystkich graczy o stanie gry
            sendWordAndPointsToClients(&*lobbyIt, &*playerIt);

            if (std::all_of(game.wordInProgress.begin(), game.wordInProgress.end(), [](char c) { return c != '_'; })) {
                game.nextRound();
                // todo sprawdzić czy to nie koniec całej gry
                if (!game.isGameActive) {
                    // gra się zakończyła, wysłać właściwy sygnał i informacje [78]
                    return;
                }
                for (const auto& player : lobbyIt->players) {
                    sendToClient(player.sockfd, "XX", game.wordInProgress);
                }
            }
        }
        else {
            // todo nie odbierać życia jeżeli litera się powtórzyła
            playerIt->lives--;
            playerIt->failedLetters.push_back(letter); // todo być może nie potrzebne
            std::string msgBody = "0," + std::string(1,letter) + "," + std::to_string(playerIt->lives);
            sendToClient(clientFd, "06", msgBody);
            sendLivesToClients(&*lobbyIt, &*playerIt);

            // sprawdzenie czy nie był to ostatni gracz, który mógł zgadywać (i właśnie nie stracił ostatniej szansy)
            if (playerIt->lives == 0) {
                // todo sprawdzić czy koniec gry (wszyscy powiesili wisielce)
                if (std::all_of(lobbyIt->players.begin(), lobbyIt->players.end(), [](const Player& player) { return player.lives <= 0; })) {
                    for (const auto& player : lobbyIt->players) {
                        sendToClient(player.sockfd, "78", "tu będzie coś");
                    }
                    game.nextRound();
                }
            }
        }
    }
    // TODO hania - dostować gui
    // else if (msg.substr(0, 2) == "07") {  // Restart gry
    //     std::string roomName = messageSubstring(msg);
    //
    //     auto lobbyIt = std::find_if(lobbies.begin(), lobbies.end(), [&roomName](const Lobby& lobby) {
    //         return lobby.name == roomName;
    //     });
    //
    //     if (lobbyIt != lobbies.end()) {
    //         auto ownerIt = std::find_if(lobbyIt->players.begin(), lobbyIt->players.end(), [](const Player& player) {
    //             return player.isOwner;  // szukamy obecnego wlasciciela lobby
    //         });
    //
    //         if (ownerIt != lobbyIt->players.end() && ownerIt->sockfd == clientFd) {
    //             std::cout << "Restart gry w pokoju: " << roomName << "\n";
    //             // restart gry w tym lobby
    //             lobbyIt->startGame();
    //             for (const auto& player : lobbyIt->players) {  // powiadomienie graczy
    //                 sendToClient(player.sockfd, "07", "1\ngameRestartSuccessful\n");
    //             }
    //         } else {
    //             sendToClient(clientFd, "07", "0\nnotLobbyLeader\n");  // Gracz nie jest liderem
    //         }
    //     } else {
    //         sendToClient(clientFd, "07", "0\ngameRestartFailed\n");  // Pokój nie istnieje
    //     }
    // }
    // // TODO hania - dostować gui
    // else if (msg.substr(0, 2) == "08") {  // Stan gry
    //     std::string roomName = messageSubstring(msg);
    //
    //
    //     auto lobbyIt = std::find_if(lobbies.begin(), lobbies.end(), [&roomName](const Lobby& lobby) {
    //         return lobby.name == roomName;
    //     });
    //
    //     if (lobbyIt != lobbies.end()) {
    //         std::string gameState = "Game state in room: " + roomName + "\n";
    //         gameState += "Current round: " + std::to_string(lobbyIt->game.currentRound) + "/" + std::to_string(lobbyIt->roundsAmount) + "\n";
    //         gameState += "Game over: " + std::string(lobbyIt->game.isGameActive ? "Yes" : "No") + "\n";
    //         gameState += "Difficulty: " + std::to_string(lobbyIt->difficulty) + "\n";
    //
    //         for (auto& player : lobbyIt->players) {
    //             gameState += player.nick + ": " + std::to_string(player.points) + "\n";
    //         }
    //
    //         gameState += "Guessing word: " + lobbyIt->game.currentWord + "\n";
    //         gameState += "Guessed letters: ";
    //         for (char letter : lobbyIt->game.wordInProgress) {
    //             gameState += letter;
    //         }
    //
    //         sendToClient(clientFd, "08", gameState);  // Stan gry
    //     } else {
    //         sendToClient(clientFd, "08", "0\ngameStateReadFailed\n");
    //     }
    // }
    // TODO hania - dostować gui
    // else if (msg.substr(0, 2) == "09") {  // Opuszczenie pokoju
    //     std::string roomName = messageSubstring(msg);
    //
    //     auto lobbyIt = std::find_if(lobbies.begin(), lobbies.end(), [&roomName](const Lobby& lobby) {
    //         return lobby.name == roomName;
    //     });
    //
    //     if (lobbyIt != lobbies.end()) {
    //         auto playerIt = std::find_if(lobbyIt->players.begin(), lobbyIt->players.end(), [clientFd](const Player& player) {
    //             return player.sockfd == clientFd;
    //         });
    //
    //         if (playerIt != lobbyIt->players.end()) {
    //             if (playerIt->isOwner) {  // jezeli byl to gracz przestaje byc ownerem
    //                 playerIt->isOwner = false;
    //             }
    //
    //             lobbyIt->joinTimes.erase(clientFd);  // usuwanie gracza z listy czasow dolaczania
    //
    //             lobbyIt->players.erase(playerIt);  // usuwanie gracza z listy graczy
    //             lobbyIt->playersCount--;
    //
    //             if (!lobbyIt->players.empty()) {
    //                 lobbyIt->setOwner();  // ustawienie nowego ownera gdyby obecny wlasnie wyszedl
    //             }
    //
    //         }
    //
    //         sendToClient(clientFd, "09", "1");  // Sukces
    //     } else {
    //         sendToClient(clientFd, "09", "0");
    //     }
    // }
    // // TODO hania zmienić numer komendy + dostosować gui
    // else if (msg.substr(0, 2) == "10") {  // Informacje o lobby
    //     auto lobbyIt = std::find_if(lobbies.begin(), lobbies.end(), [clientFd](const Lobby& lobby) {
    //         return std::any_of(lobby.players.begin(), lobby.players.end(), [clientFd](const Player& player) {
    //             return player.sockfd == clientFd;
    //         });
    //     });
    //
    //     if (lobbyIt != lobbies.end()) {
    //         auto ownerIt = std::find_if(lobbyIt->players.begin(), lobbyIt->players.end(),
    //             [](const Player& player) { return player.isOwner; });
    //         std::string ownerNick;
    //
    //         if (ownerIt != lobbyIt->players.end()) {
    //             ownerNick = ownerIt->nick;
    //         } else {
    //             ownerNick = "None";
    //         }
    //         std::string response = "10\nroomFound\n" + lobbyIt->name +
    //                             "\nPlayers: " + std::to_string(lobbyIt->playersCount) +
    //                             "\nOwner: " + ownerNick + "\n";
    //         sendToClient(clientFd, "10", response);
    //     } else {
    //         sendToClient(clientFd, "10", "0");
    //     }
    // }
    else if (msg.substr(0, 2) == "70") { // prośba o listę pokoi (dla jednego gracza)
        sendLobbiesToClients(lobbyNames, clientFd);
    }
}

