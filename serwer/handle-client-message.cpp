#include "handle-client-message.h"

extern std::vector<std::shared_ptr<Player>> players;
extern std::vector<std::shared_ptr<Lobby>> lobbies;
extern std::vector<std::string> playersNicknames;
extern std::vector<std::string> lobbyNames;
extern int lobbyCount;

void handleClientMessage(int clientFd, const std::string& msg) {
    if (msg.substr(0, 2) == "01") { // Ustawienie nicku
        std::string nick = messageSubstring(msg);

        if (std::find(playersNicknames.begin(), playersNicknames.end(), nick) != playersNicknames.end()) {
            std::cout << "Nick, " << nick << ", has already been taken.\n";
            sendToClient(clientFd, "01", "01");
            return;
        }

        auto playerIt = std::find_if(players.begin(), players.end(), [clientFd](const std::shared_ptr<Player>& player) {
           return player->sockfd == clientFd;
        });
        if (playerIt == players.end()) {
            std::cout << "Player's socket has not been found in the players vector (socket: " << clientFd << ")";
            sendToClient(clientFd, "01", "02");
            return;
        }

        // SUCCESS
        (*playerIt)->nick = nick;
        playersNicknames.push_back(nick);
        sendToClient(clientFd, "01", "1");
    }
    else if (msg.substr(0, 2) == "02") {  // Tworzenie pokoju
        std::string settings = messageSubstring(msg);
        Settings parsedSettings = parseSettings(settings);

        std::string lobbyName = parsedSettings.name;

        auto lobbyIt = std::find_if(lobbies.begin(), lobbies.end(), [&lobbyName](const std::shared_ptr<Lobby>& lobby) {
           return lobby->name == lobbyName;
        });
        // todo dodać wszędzie sprawdzanie nullptr
        if (lobbyIt != lobbies.end() && *lobbyIt != nullptr) {
            sendToClient(clientFd, "02", "01");
            std::cout << "Failed to create lobby. Name already taken: " << lobbyName << "\n";
            return;
        }
        auto playerIt = std::find_if(players.begin(), players.end(), [clientFd](const std::shared_ptr<Player>& player) {
           return player->sockfd == clientFd;
        });
        if (playerIt == players.end() && *playerIt != nullptr) {
            sendToClient(clientFd, "02", "02"); // Nie znaleziono gracza
            return;
        }

        auto newLobby = std::make_shared<Lobby>();
        newLobby->name = lobbyName;
        newLobby->password = parsedSettings.password;
        newLobby->difficulty = parsedSettings.difficulty;
        newLobby->roundsAmount = parsedSettings.roundsAmount;
        newLobby->roundDuration = parsedSettings.roundDurationSec;

        // SUCCESS
        newLobby->players.push_back(*playerIt);
        newLobby->playersCount++;
        newLobby->setOwner();

        lobbyCount++;
        lobbyNames.push_back(lobbyName);

        lobbies.push_back(newLobby);

        sendLobbiesToClients(lobbyNames);
        sendPlayersToClients(newLobby.get());

        sendToClient(clientFd, "02", "1");
        (*playerIt)->isReadyToPlay = true;
        (*playerIt)->lobbyName = lobbyName;
        std::cout << "Lobby created successfully: " << lobbyName << "\n";
        std::cout << "Current lobby count: " << lobbyCount << "\n";
    }
    else if (msg.substr(0, 2) == "03") {  // Dołączanie do pokoju
        std::string lobbyInfo = messageSubstring(msg);

        // PARSOWANIE
        size_t posName = msg.find("name:");
        size_t posPass = msg.find("password:");

        size_t nameStart = posName + strlen("name:");
        size_t nameEnd = msg.find(',', nameStart);
        std::string lobbyName = msg.substr(nameStart, nameEnd - nameStart);

        size_t passStart = posPass + strlen("password:");
        size_t passEnd = msg.find(',', passStart);
        std::string password = msg.substr(passStart, passEnd - passStart);

        auto lobbyIt = std::find_if(lobbies.begin(), lobbies.end(), [&lobbyName](const std::shared_ptr<Lobby>& lobby) {
            return lobby->name == lobbyName;
        });
        if (lobbyIt == lobbies.end()) {
            sendToClient(clientFd, "03", "04");  // pokój nie istnieje
            return;
        }

        auto playerIt = std::find_if(players.begin(), players.end(), [clientFd](const std::shared_ptr<Player>& player) {
            return player->sockfd == clientFd;
        });
        if (playerIt == players.end()) {
            sendToClient(clientFd, "03", "05");  // Nie znaleziono gracza
            return;
        }


        // FAIL
        if (password != (*lobbyIt)->password) {
            sendToClient(clientFd, "03", "01");  // niepoprawne hasło
            return;
        }
        if ((*lobbyIt)->playersCount >= MAXPLAYERS) {
            sendToClient(clientFd, "03", "02"); // osiągnięto max ilość graczy
            return;
        }
        if ((*lobbyIt)->game.isGameActive) {
            sendToClient(clientFd, "03", "03"); // trwa teraz gra
            return;
        }

        // SUCCESS
        (*lobbyIt)->players.push_back(*playerIt);
        (*lobbyIt)->playersCount++;
        sendToClient(clientFd, "03", "1");
        (*playerIt)->isReadyToPlay = true;
        (*playerIt)->lobbyName = lobbyName;
        sendPlayersToClients(lobbyIt->get());
        isStartAllowed(lobbyIt->get());
    }
    else if (msg.substr(0, 2) == "73") {  // Start gry
        auto lobbyIt = std::find_if(lobbies.begin(), lobbies.end(), [clientFd](const std::shared_ptr<Lobby>& lobby) {
        return !lobby->players.empty() && lobby->owner->sockfd == clientFd;
            });
        if (lobbyIt == lobbies.end()) {
            sendToClient(clientFd, "73", "0");  // pokój nie istnieje
            return;
        }


        // SUCCESS
        (*lobbyIt)->startGame();
        sendStartToClients(lobbyIt->get());
    }
    else if (msg.substr(0, 2) == "06") {  // Próba zgadnięcia litery
        char letter = messageSubstring(msg)[0];

        auto lobbyIt = std::find_if(lobbies.begin(), lobbies.end(), [clientFd](const std::shared_ptr<Lobby>& lobby) {
            return std::any_of(lobby->players.begin(), lobby->players.end(), [clientFd](const std::shared_ptr<Player>& player) {
                return player->sockfd == clientFd;
            });
        });
        if (lobbyIt == lobbies.end()) {
            sendToClient(clientFd, "06", "01"); // nie znaleziono lobby
            return;
        }
        auto& game = (*lobbyIt)->game;

        auto playerIt = std::find_if((*lobbyIt)->players.begin(), (*lobbyIt)->players.end(), [clientFd](const std::shared_ptr<Player>& player) {
            return player->sockfd == clientFd;
        });
        if (playerIt == (*lobbyIt)->players.end()) {
            sendToClient(clientFd, "06", "02"); // nie znaleziono gracza
            return;
        }

        if ((*playerIt)->lives == 0) {
            sendToClient(clientFd, "06", "03"); // koniec żyć (dla pewności)
            return;
        }

        // todo dlaczego char nie działa a string tak?

        if (!game.guessedLetters.empty()) {
            if (std::find(game.guessedLetters.begin(), game.guessedLetters.end(), letter) != game.guessedLetters.end()) {
                sendToClient(clientFd, "06", "2," + std::string(1,letter)); // litera już zgadnięta
                return;
            }
        }
        if (!(*playerIt)->failedLetters.empty()) {
            if (std::find((*playerIt)->failedLetters.begin(), (*playerIt)->failedLetters.end(), letter) != (*playerIt)->failedLetters.end()) {
                sendToClient(clientFd, "06", "3," + std::string(1,letter)); // litera została już wcześniej wykorzystana jako fail
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
            (*playerIt)->points += 25 * occurrences;

            // 3. wysłanie graczowi odpowiedzi na jego literę
            std::string msgBody = "1," + std::string(1,letter); //todo sprawdź czemu tu char nie działa
            sendToClient(clientFd, "06", msgBody);

            // 4. powiadomienie wszystkich graczy o stanie gry
            sendWordAndPointsToClients(lobbyIt->get(), playerIt->get());

            if (game.wordInProgress == game.currentWord) {
                std::cout << "HASŁO ZGADNIĘTE w lobby: " + (*lobbyIt)->name + "\n";
                game.nextRound();
                if (!game.isGameActive) {
                    sendEndToClients(lobbyIt->get()); // 78 - koniec gry
                    return;
                }
                for (auto& player : (*lobbyIt)->players) {
                    player->failedLetters.clear();
                    sendToClient(player->sockfd, "79", game.wordInProgress + "," + std::to_string((*lobbyIt)->game.currentRound) + "," + std::to_string((*lobbyIt)->roundsAmount));
                }
            }
        }
        else {
            (*playerIt)->lives--;
            (*playerIt)->failedLetters.push_back(letter);
            std::string msgBody = "0," + std::string(1,letter) + "," + std::to_string((*playerIt)->lives);
            sendToClient(clientFd, "06", msgBody);
            sendLivesToClients(lobbyIt->get(), playerIt->get());

            // sprawdzenie czy nie był to ostatni gracz, który mógł zgadywać (i właśnie nie stracił ostatniej szansy)
            if ((*playerIt)->lives == 0) {
                if (std::all_of((*lobbyIt)->players.begin(), (*lobbyIt)->players.end(), [](const std::shared_ptr<Player>& player) {
                    return player->lives <= 0;
                })) {
                    game.nextRound();
                    if (!game.isGameActive) {
                        sendEndToClients(lobbyIt->get()); // 78 - koniec gry
                        return;
                    }
                    for (auto& player : (*lobbyIt)->players) {
                        player->failedLetters.clear();
                        sendToClient(player->sockfd, "79", game.wordInProgress + "," + std::to_string((*lobbyIt)->game.currentRound) + "," + std::to_string((*lobbyIt)->roundsAmount));
                    }
                }
            }
        }
    }
    else if (msg.substr(0, 2) == "80") {  // czas się skończył, nowa runda
        auto lobbyIt = std::find_if(lobbies.begin(), lobbies.end(), [clientFd](const std::shared_ptr<Lobby>& lobby) {
            return !lobby->players.empty() && lobby->owner->sockfd == clientFd;
        });

        if (lobbyIt != lobbies.end()) {
            std::cout << "CZAS MINĄŁ w lobby: " + (*lobbyIt)->name + "\n";
            (*lobbyIt)->game.nextRound();
            if (!(*lobbyIt)->game.isGameActive) {
                sendEndToClients(lobbyIt->get()); // 78 - koniec gry
                return;
            }
            for (auto& player : (*lobbyIt)->players) {
                player->failedLetters.clear();
                sendToClient(player->sockfd, "79", (*lobbyIt)->game.wordInProgress + "," + std::to_string((*lobbyIt)->game.currentRound) + "," + std::to_string((*lobbyIt)->roundsAmount));
            }
        }
        else {
            sendToClient(clientFd, "79", "0");  // Pokój nie istnieje
        }
    }
    else if (msg.substr(0, 2) == "09") {  // Opuszczenie pokoju
        auto lobbyIt = std::find_if(lobbies.begin(), lobbies.end(), [clientFd](const std::shared_ptr<Lobby>& lobby) {
            return std::any_of(lobby->players.begin(), lobby->players.end(), [clientFd](const std::shared_ptr<Player>& player) {
                return player->sockfd == clientFd;
            });
        });


        if (lobbyIt == lobbies.end()) {
            sendToClient(clientFd, "09", "0");
        }

        auto playerIt = std::find_if((*lobbyIt)->players.begin(), (*lobbyIt)->players.end(), [clientFd](const std::shared_ptr<Player>& player) {
            return player->sockfd == clientFd;
        });

        if (playerIt != (*lobbyIt)->players.end()) {
            std::string nick = (*playerIt)->nick;

            (*playerIt)->isOwner = false;
            (*playerIt)->lobbyName = "";

            (*lobbyIt)->players.erase(playerIt);
            (*lobbyIt)->playersCount--;

            if ((*lobbyIt)->game.isGameActive) {
                for (const auto& player : (*lobbyIt)->players) {
                    sendToClient(player->sockfd, "81", nick);
                }
            }
            else {
                isStartAllowed(lobbyIt->get());
                sendPlayersToClients(lobbyIt->get());
            }

            sendToClient(clientFd, "09", "1");
            // todo sprawdzić czy tu nie trzeba usunąć pokoju bo a) podczas gry został jeden gracz b) w pokoju bez gry jest 0 graczy
        }
        else {
            sendToClient(clientFd, "09", "0");
        }
    }
    else if (msg.substr(0, 2) == "70") { // prośba o listę pokoi (dla jednego gracza)
        sendLobbiesToClients(lobbyNames, clientFd);
    }
    else if (msg.substr(0, 2) == "71") { // prośba o listę graczy
        auto lobbyIt = std::find_if(lobbies.begin(), lobbies.end(), [clientFd](const std::shared_ptr<Lobby>& lobby) {
            return std::any_of(lobby->players.begin(), lobby->players.end(), [clientFd](const std::shared_ptr<Player>& player) {
                return player->sockfd == clientFd;
            });
        });
        if (lobbyIt == lobbies.end()) {
            sendToClient(clientFd, "71", "01"); // nie znaleziono lobby
            return;
        }

        sendPlayersToClients(lobbyIt->get(), -1, clientFd);
    }
    else if (msg.substr(0, 2) == "82") { // gracz gotowy do rozpoczęcia gry
        //todo check this out:
        // auto lobbyIt = std::find_if(lobbies.begin(), lobbies.end(), [clientFd](Lobby& lobby) {
        //     auto playerIt = std::find_if(lobby.players.begin(), lobby.players.end(), [clientFd](Player* player) {
        //         return player->sockfd == clientFd;
        //     });
        //     return playerIt != lobby.players.end();
        // });
        std::string state = messageSubstring(msg);

        auto playerIt = std::find_if(players.begin(), players.end(), [clientFd](const std::shared_ptr<Player>& player) {
            return player->sockfd == clientFd;
        });
        if (playerIt == players.end()) {
            std::cout << "Player's socket has not been found in the players vector (socket: " << clientFd << ")";
            sendToClient(clientFd, "82", "02");
            return;
        }

        if ((*playerIt)->lobbyName.empty()) {
            std::cout << "jestem w return (ten player nie ma przypisanego lobby w struct Player)\n";
            return;
        }

        auto lobbyIt = std::find_if(lobbies.begin(), lobbies.end(), [clientFd](const std::shared_ptr<Lobby>& lobby) {
            return std::any_of(lobby->players.begin(), lobby->players.end(), [clientFd](const std::shared_ptr<Player>& player) {
                return player->sockfd == clientFd;
            });
        });
        if (lobbyIt == lobbies.end()) {
            sendToClient(clientFd, "82", "01"); // nie znaleziono lobby
            return;
        }
        //todo sprawdzić
        auto& game = (*lobbyIt)->game;

        if (state == "1") {
            (*playerIt)->isReadyToPlay = true;
        }
        else {
            (*playerIt)->isReadyToPlay = false;
        }
        isStartAllowed(lobbyIt->get());
    }
}

