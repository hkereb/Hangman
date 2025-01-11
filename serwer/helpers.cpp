#include "helpers.h"

extern std::vector<Player> players;
extern std::vector<Lobby> lobbies;
extern std::vector<std::string> lobbyNames;

void sendLobbiesToClients(std::vector<std::string> lobbyNames, int clientFd) {
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

void sendPlayersToClients(const Lobby* lobby, int ignoreFd, int onlyFd) {
    std::string msgBody;

    // przygotowanie wiadomości
    for (size_t i = 0; i < lobby->players.size(); ++i) {
        if (lobby->players[i]->sockfd != ignoreFd) {
            msgBody += lobby->players[i]->nick;
            if (i != lobby->players.size() - 1) {
                msgBody += ",";
            }
        }
    }

    if (onlyFd != -1) {
        sendToClient(onlyFd, "71", msgBody);
    }

    // wysłanie do wszystkich klientów z pokoju
    for (const auto& player : lobby->players) {
        if (player->sockfd != ignoreFd) {
            sendToClient(player->sockfd, "71", msgBody);
        }
    }
}

// update rankingu i hasła dla WSZYSTKICH z pokoju
void sendWordAndPointsToClients(const Lobby* lobby, const Player* playerWhoGuessed) {
    std::string msgWord = lobby->game.wordInProgress;
    std::string msgPoints = playerWhoGuessed->nick + "," + std::to_string(playerWhoGuessed->points);

    // wysłanie do wszystkich klientów z pokoju
    for (const auto& player : lobby->players) {
        sendToClient(player->sockfd, "75", msgWord);
        sendToClient(player->sockfd, "77", msgPoints);
    }
}

// update wisielca PRZECIWNIKA dla WSZYSTKICH graczy z pokoju oprócz niego
void sendLivesToClients(const Lobby* lobby, const Player* playerWhoMissed) {
    std::string msgLives = playerWhoMissed->nick + "," + std::to_string(playerWhoMissed->lives);

    // wysłanie do wszystkich klientów z pokoju oprócz tego gracza
    for (const auto& player : lobby->players) {
        if (player->nick != playerWhoMissed->nick) {
               sendToClient(player->sockfd, "76", msgLives);
        }
    }
}

void sendStartToClients(const Lobby* lobby) {
    for (const auto& player : lobby->players) {
        std::string time = Game::convertTime(lobby->roundDuration);
        std::string msgBody = "1;" + lobby->game.wordInProgress + ";" + time + ";"+ std::to_string(lobby->roundsAmount) + ";" + player->nick + ";";
        int count = 0;

        for (const auto& opponent : lobby->players) {
            if (opponent->nick != player->nick) {
                count++;
                if (count != 1) {
                    msgBody += ",";
                }
                msgBody += opponent->nick;
            }
        }

        sendToClient(player->sockfd, "73", msgBody);
    }
}

void sendEndToClients(const Lobby* lobby) {
    std::string msgBody;
    int count1 = 0;
    for (const auto& player : lobby->players) {
        count1++;
        if (count1 != 1) {
            msgBody += ",";
        }
        msgBody += player->nick + ":" + std::to_string(player->points);
    }
    msgBody += ";";

    int count2 = 0;
    for (const auto& word : lobby->game.wordList) {
        count2++;
        if (count2 != 1) {
            msgBody += ",";
        }
        msgBody += word;
    }

    for (const auto& player : lobby->players) {
        sendToClient(player->sockfd, "78", msgBody);
    }
}

void isStartAllowed(const Lobby* lobby) {
    if (lobby->game.isGameActive) {
        std::cout << "i think game is active";
        return;
    }
    if (lobby->playersCount < 2) {
        sendToClient(lobby->players[0]->sockfd, "72", "01"); // brakuje graczy
        return;
    }
    for (const Player* player : lobby->players) {
        if (!player->isReadyToPlay) {
            sendToClient(lobby->players[0]->sockfd, "72", "02"); // nie wszyscy gracze są gotowi (nie wrócili po grze do waitroom)
            return;
        }
    }
    sendToClient(lobby->players[0]->sockfd, "72", "1");
}

Settings parseSettings(const std::string& msg) {
    Settings settings;

    const size_t posName = msg.find("name:");
    const size_t posPass = msg.find("password:");
    const size_t posDiff = msg.find("difficulty:");
    const size_t posRounds = msg.find("rounds:");
    const size_t posTime = msg.find("time:");

    const size_t nameStart = posName + strlen("name:"); // po "name:"
    const size_t nameEnd = msg.find(',', nameStart); // do przecinka
    settings.name = msg.substr(nameStart, nameEnd - nameStart);

    const size_t passStart = posPass + strlen("password:");
    const size_t passEnd = msg.find(',', passStart);
    settings.password = msg.substr(passStart, passEnd - passStart);

    const size_t diffStart = posDiff + strlen("difficulty:");
    const size_t diffEnd = msg.find(',', diffStart);
    settings.difficulty = std::stoi(msg.substr(diffStart, diffEnd - diffStart));

    const size_t roundsStart = posRounds + strlen("rounds:");
    const size_t roundsEnd = msg.find(',', roundsStart);
    settings.roundsAmount = std::stoi(msg.substr(roundsStart, roundsEnd - roundsStart));

    const size_t timeStart = posTime + strlen("time:");
    const size_t timeEnd = msg.find(',', timeStart);
    settings.roundDurationSec = std::stoi(msg.substr(timeStart, timeEnd - timeStart));

    return settings;
}

std::string messageSubstring(std::string msg) {
    return msg.substr(3);
}

void removeFromLobby(int clientFd) {
    for (auto & lobby : lobbies) {
        auto playerIt = std::find_if(lobby.players.begin(), lobby.players.end(), [clientFd](const Player* player) {
            return player->sockfd == clientFd;
        });

        if (playerIt != lobby.players.end()) {
            Player* playerToRemove = *playerIt;
            playerToRemove->lobbyName = "";
            std::cout << "Player: " << playerToRemove->nick << ", got removed from lobby: " << lobby.name << "\n";
            lobby.playersCount--;
            if (lobby.game.isGameActive) { // gra trwa (game page)
                auto& gamePlayers = lobby.game.players;
                auto gamePlayerIt = std::find_if(gamePlayers.begin(), gamePlayers.end(), [clientFd](const Player* player) {
                    return player->sockfd == clientFd;
                });

                if (gamePlayerIt != gamePlayers.end()) {
                    std::cout << "Removing player " << (*gamePlayerIt)->nick << " from game.\n";
                    gamePlayers.erase(gamePlayerIt); // Usuwamy gracza z gry
                }

                for (auto & player : lobby.players) {
                    if (playerToRemove->nick != player->nick) {
                        sendToClient(player->sockfd, "74", playerToRemove->nick);
                    }
                }
            }
            else { // gra nie trwa (waitroom lub end page)
                sendPlayersToClients(&lobby, playerToRemove->sockfd);
                isStartAllowed(&lobby);
            }
            lobby.players.erase(playerIt);

            break;
        }
    }
}

