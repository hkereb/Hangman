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
#include <vector>
#include <algorithm>
#include <sstream>
#include <map>
#include <random>

void sendToClient(int client_fd, const std::string& command_number, const std::string& body);

#define MAXEPOLLSIZE 1000
#define BACKLOG 200 // how many pending connections queue will hold
#define MAXPLAYERS 4

struct Player {
    int sockfd;
    std::string nick;
};

std::vector<Player> players;

struct Game {
    std::vector<std::string> wordList;
    std::string currentWord;
    std::vector<char> guessedLetters;
    std::vector<Player> players;
    int roundDuration;
    int currentRound;
    int roundsAmount;
    int difficulty;
    bool isGameOver;

    Game(int roundsAmount, int roundDuration, int difficulty) {
        this->roundsAmount = roundsAmount;
        this->roundDuration = roundDuration;
        this->currentRound = 0;
        this->difficulty = 1;
        this->isGameOver = false;
        
        initializeWordList();
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
        nextRound();  
    }

    void nextRound() {
        if (currentRound >= roundsAmount) {
            isGameOver = true;  // zakonczenie gry
            return;
        }

        currentRound++;  // zwiekszenie numeru rundy

        // ustawienie slowa do odgadniecia na aktualna runde
        currentWord = wordList[currentRound - 1];  
        guessedLetters.resize(currentWord.size(), '_');  // resetowanie odgadnietych liter
    }
};

struct Lobby {
    std::string name;
    std::string password;
    int count_players;
    std::vector<Player> players;

    int difficulty;
    int roundsAmount;
    int roundDuration;

    void startGame() {
        if (players.size() < 2) { // w lobby musi byc minimalnie 2 graczy
            std::cout << "Potrzeba przynajmniej 2 graczy w lobby\n";
            return;
        }

        Game new_game(roundsAmount, roundDuration, difficulty);
        new_game.players = players;
        new_game.startGame();

        std::cout << "Gra rozpoczęta w lobby: " << name << "\n";

        for (const auto& player : players) {
            sendToClient(player.sockfd, "05", "1\ngameStarted\n");
        }
    }
};


std::list<Lobby> gameLobbies;
int lobbyCount = 0;

std::vector<std::string> client_nicks;

int setNonBlocking(int sockfd);
int startListening();

// funkcja do mapowania ustawien w formie difficulty=3&roundDuration=120 itd.
std::map<std::string, int> parseSettings(const std::string& settings) {
    std::map<std::string, int> parsedSettings;
    std::istringstream ss(settings);
    std::string pair;

    // Funkcja walidacji dla kluczy, które wymagają sprawdzenia
    auto validateSetting = [](const std::string& key, const std::string& value) -> bool {
        if (key == "roundsAmount" || key == "roundDuration") {
            // Sprawdzenie, czy wartość to liczba całkowita i większa od zera
            try {
                int num = std::stoi(value);
                return num > 0;
            } catch (...) {
                return false;
            }
        }
        // Inne klucze, np. difficulty, nie wymagają walidacji
        return true;
    };

    while (std::getline(ss, pair, '&')) {
        size_t pos = pair.find('=');
        if (pos != std::string::npos) {
            std::string key = pair.substr(0, pos);
            std::string value = pair.substr(pos + 1);

            // Jeśli wartość jest poprawna lub klucz nie wymaga walidacji
            if (validateSetting(key, value)) {
                if (key == "difficulty" || key == "roundsAmount" || key == "roundDuration") {
                    parsedSettings[key] = std::stoi(value);  // Konwersja do liczby całkowitej
                } else {
                    std::cout << "Wrong setting" << std::endl;
                }
            } else {
                std::cout << "Invalid setting: " << key << "=" << value << ". Using default values.\n";
                // Ustawienia domyślne
                if (key == "roundsAmount") {
                    parsedSettings[key] = 5;  // Domyślna liczba rund
                } else if (key == "roundDuration") {
                    parsedSettings[key] = 60; // Domyślny czas rundy (60 sekund)
                } else if (key == "difficulty") {
                    parsedSettings[key] = 1;
                }
            }
        }
    }

    return parsedSettings;
}

void sendToClient(int client_fd, const std::string& command_number, const std::string& body) {
    if (command_number.size() != 2) {
        std::cerr << "Error: Command number must consist of 2 characters";
        return;
    }

    std::string full_message = command_number + body;
    ssize_t bytes_sent = send(client_fd, full_message.c_str(), full_message.size(), 0);

    if (bytes_sent == -1) {
        std::cerr << "Error: Failed to send message to client with socket " << client_fd << ".\n";
    }
}

// obsługa klienta na podstawie jego wiadomości
void handle_client_message(int client_fd, std::string msg) {
    if (msg.substr(0, 2) == "01") {
        std::string nick = msg.substr(2);

        if (std::find(client_nicks.begin(), client_nicks.end(), nick) != client_nicks.end()) {
            std::cout << "Nick, " << nick << ", has already been taken.\n";
            // powiadomienie strony klienta o niepowodzeniu
            sendToClient(client_fd, "01", "0\nnickTaken\n");
            return;
        }

        auto player_it = std::find_if(players.begin(), players.end(), [client_fd](const Player& player) {
            return player.sockfd == client_fd;
        });

        if (player_it != players.end()) {
            player_it->nick = nick;
            client_nicks.push_back(nick);
            sendToClient(client_fd, "01", "1\nnickAccepted\n");
            //std::cout << "Player's accepted nickname: " << it->nick;
        } else {
            std::cout << "Player's socket has not been found in the players vector (socket: " << client_fd << ")";
            sendToClient(client_fd, "01", "0\nnickAssignmentFailed\n");
            return;
        }
    } else if (msg.substr(0, 2) == "02") {
        std::string lobby_name = msg.substr(2);

        // Check if a lobby with the same name already exists
        auto lobby_it = std::find_if(gameLobbies.begin(), gameLobbies.end(), [&lobby_name](const Lobby& lobby) {
            return lobby.name == lobby_name;
        });

        if (lobby_it != gameLobbies.end()) {
            // Lobby name already taken
            sendToClient(client_fd, "02", "0\nlobbyNameTaken\n");  // Send failure response
            std::cout << "Failed to create lobby. Name already taken: " << lobby_name << "\n";
            return;
        }

        // Create a new lobby
        Lobby new_lobby;
        new_lobby.name = lobby_name;
        new_lobby.password = "";  // todo: password handling
        new_lobby.count_players = 0;

        // lokalizowanie gracza tworzacego lobby
        auto player_it = std::find_if(players.begin(), players.end(), [client_fd](const Player& player) {
            return player.sockfd == client_fd;
        });

        if (player_it != players.end()) {
            // Add the player to the lobby
            new_lobby.players.push_back(*player_it);
            new_lobby.count_players++;
        }

        // Add the lobby to the list of lobbies
        gameLobbies.push_back(new_lobby);
        lobbyCount++;

        sendToClient(client_fd, "02", "1\nlobbyCreated\n");  // stworzono lobby
        std::cout << "Lobby created successfully: " << lobby_name << "\n";
        std::cout << "Current lobby count: " << lobbyCount << "\n";
    } else if (msg.substr(0, 2) == "03") {  // Dołączanie do pokoju
        std::string room_name = msg.substr(2);

        // Znajdź pokój
        auto lobby_it = std::find_if(gameLobbies.begin(), gameLobbies.end(), [&room_name](const Lobby& lobby) {
            return lobby.name == room_name;
        });

        if (lobby_it == gameLobbies.end()) {
            sendToClient(client_fd, "03", "0\nroomNotExists\n");  // Pokój nie istnieje
            return;
        }

        // sprawdzenie czy gracz juz jest w pokoju
        auto player_it = std::find_if(lobby_it->players.begin(), lobby_it->players.end(), [client_fd](const Player& player) {
            return player.sockfd == client_fd;
        });

        // powiadomienie ze gracz jest juz w pokoju
        if (player_it != lobby_it->players.end()) {
            sendToClient(client_fd, "03", "0\nalreadyInRoom\n");
            return;
        }

        // powiadomienie ze pokoj jest pelny
        if (lobby_it->count_players >= MAXPLAYERS) {
            sendToClient(client_fd, "03", "0\nroomIsFull\n"); 
            return;
        }
        
        // wyszukanie gracza ktory ma dolaczyc do pokoju
        player_it = std::find_if(players.begin(), players.end(), [client_fd](const Player& player) {
            return player.sockfd == client_fd;
        });
        
        if (player_it != players.end()) {
            lobby_it->players.push_back(*player_it);  // Dodaj gracza do pokoju
            lobby_it->count_players++;
            sendToClient(client_fd, "03", "1\njoinedRoom\n");  // Sukces
        } else {
            sendToClient(client_fd, "03", "0\nplayerNotFound\n");  // Nie znaleziono gracza
        }
    } else if (msg.substr(0, 2) == "04") {  // Ustawienia pokoju
        std::string settings = msg.substr(2);

        // parsowanie stringa
        auto parsedSettings = parseSettings(settings);

        // Find the lobby for the client
        auto lobby_it = std::find_if(gameLobbies.begin(), gameLobbies.end(), [client_fd](const Lobby& lobby) {
            return std::any_of(lobby.players.begin(), lobby.players.end(), [client_fd](const Player& player) {
                return player.sockfd == client_fd;
            });
        });

        if (lobby_it != gameLobbies.end()) {
            // przypisanie ustawien
            if (parsedSettings.find("difficulty") != parsedSettings.end()) {
                lobby_it->difficulty = parsedSettings["difficulty"];
            }
            if (parsedSettings.find("roundsAmount") != parsedSettings.end()) {
                lobby_it->roundsAmount = parsedSettings["roundsAmount"];
            }
            if (parsedSettings.find("roundDuration") != parsedSettings.end()) {
                lobby_it->roundDuration = parsedSettings["roundDuration"];
            }

            // wypisanie ustawien
            std::cout << "Updated settings for lobby " << lobby_it->name << ":\n";
            std::cout << "  Difficulty: " << lobby_it->difficulty << "\n";
            std::cout << "  Rounds: " << lobby_it->roundsAmount << "\n";
            std::cout << "  Duration: " << lobby_it->roundDuration << "\n";

            sendToClient(client_fd, "04", "1\nsettingsAssignmentSuccessful\n");  // Success
        } else {
            sendToClient(client_fd, "04", "0\nsettingsAssignmentFailed\n");  // Failed
        }
    } else if (msg.substr(0, 2) == "05") {  // Start gry
        auto lobby_it = std::find_if(gameLobbies.begin(), gameLobbies.end(), [client_fd](const Lobby& lobby) {
            return std::any_of(lobby.players.begin(), lobby.players.end(), [client_fd](const Player& player) {
                return player.sockfd == client_fd;
            });
        });

        if (lobby_it != gameLobbies.end()) {
            // Sprawdzenie, czy gracz ma odpowiednie uprawnienia (np. lider pokoju)
            // Zazwyczaj liderem jest pierwszy gracz, który stworzył lobby
            if (lobby_it->players.front().sockfd == client_fd) {
                // Rozpoczęcie gry w tym lobby
                lobby_it->startGame();  // Wywołanie funkcji startGame
            } else {
                sendToClient(client_fd, "05", "0\nnotLobbyLeader\n");  // Gracz nie jest liderem
            }
        } else {
            sendToClient(client_fd, "05", "0\ngameStartFailed\n");  // Pokój nie istnieje
        }
    } else if (msg.substr(0, 2) == "06") {  // Próba zgadnięcia litery
        char letter = msg[2];

        auto lobby_it = std::find_if(gameLobbies.begin(), gameLobbies.end(), [client_fd](const Lobby& lobby) {
            return std::any_of(lobby.players.begin(), lobby.players.end(), [client_fd](const Player& player) {
                return player.sockfd == client_fd;
            });
        });

        if (lobby_it != gameLobbies.end()) {
            // Logika przetwarzania zgadywania litery
            std::cout << "Gracz zgadł literę: " << letter << " w pokoju: " << lobby_it->name << "\n";
            sendToClient(client_fd, "06", "1\nletterFound\n");  // Sukces
        } else {
            sendToClient(client_fd, "06", "0\nletterGuessFailed\n");  
        }
    } else if (msg.substr(0, 2) == "07") {  // Restart gry
        std::string room_name = msg.substr(2);

        auto lobby_it = std::find_if(gameLobbies.begin(), gameLobbies.end(), [&room_name](const Lobby& lobby) {
            return lobby.name == room_name;
        });

        if (lobby_it != gameLobbies.end()) {
            std::cout << "Restart gry w pokoju: " << room_name << "\n";
            sendToClient(client_fd, "07", "1\ngameRestartSuccessful\n");  // Sukces
        } else {
            sendToClient(client_fd, "07", "0\ngameRestartFailed\n");  
        }
    } else if (msg.substr(0, 2) == "08") {  // Stan gry
        std::string room_name = msg.substr(2);

        auto lobby_it = std::find_if(gameLobbies.begin(), gameLobbies.end(), [&room_name](const Lobby& lobby) {
            return lobby.name == room_name;
        });

        if (lobby_it != gameLobbies.end()) {
            std::string game_state = "Aktualny stan gry w pokoju " + room_name;
            sendToClient(client_fd, "08", game_state);  // Stan gry
        } else {
            sendToClient(client_fd, "08", "0\ngameStateReadFailed\n");
        }
    } else if (msg.substr(0, 2) == "09") {  // Opuszczenie pokoju
        std::string room_name = msg.substr(2);

        auto lobby_it = std::find_if(gameLobbies.begin(), gameLobbies.end(), [&room_name](const Lobby& lobby) {
            return lobby.name == room_name;
        });

        if (lobby_it != gameLobbies.end()) {
            lobby_it->players.erase(std::remove_if(lobby_it->players.begin(), lobby_it->players.end(), [client_fd](const Player& player) {
                return player.sockfd == client_fd;
            }), lobby_it->players.end());
            lobby_it->count_players--;
            sendToClient(client_fd, "09", "1\nplayerLeftRoomSuccessfully\n");  // Sukces
        } else {
            sendToClient(client_fd, "09", "0\nplayerLeftRoomFailed\n");  
        }
    } else if (msg.substr(0, 2) == "10") {  // Sprawdzanie pokoju i liczby graczy
        auto lobby_it = std::find_if(gameLobbies.begin(), gameLobbies.end(), [client_fd](const Lobby& lobby) {
            return std::any_of(lobby.players.begin(), lobby.players.end(), [client_fd](const Player& player) {
                return player.sockfd == client_fd;
            });
        });

        if (lobby_it != gameLobbies.end()) {
            // Zwróć nazwę pokoju oraz liczbę graczy
            std::string response = "10\nroomFound\n" + lobby_it->name + "\nPlayers: " + std::to_string(lobby_it->count_players) + "\n";
            sendToClient(client_fd, "10", response);  // Success
        } else {
            sendToClient(client_fd, "10", "0\ngettingResponseFailed\n"); 
        }
    }
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

    int fds_to_watch = 1;
    struct epoll_event *events;
    events = (struct epoll_event *)malloc(sizeof(struct epoll_event) * fds_to_watch);

    while (1) {  // loop for accepting incoming connections
        int ready = epoll_wait(efd, events, fds_to_watch, -1);
        if (ready == -1) {
            perror("epoll_wait");
            break;
        }

        for (int n = 0; n < ready; ++n) {
            if (events[n].data.fd == sockfd) {
                // std::cout << "sockfd" << std::endl;
                sockaddr_in client_addr{};
                socklen_t addr_size = sizeof(client_addr);
                int new_fd = accept(events[n].data.fd, (struct sockaddr *)&client_addr, &addr_size);
                if (new_fd == -1) {
                    if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                        break;
                    } else {
                        perror("accept");
                        break;
                    }
                }

                printf("server: new connection established.\n");
                setNonBlocking(new_fd);
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = new_fd;
                if (epoll_ctl(efd, EPOLL_CTL_ADD, new_fd, &ev) < 0) {
                    printf("Failed to insert socket into epoll.\n");
                }
                fds_to_watch++;

                Player new_player;
                new_player.sockfd = new_fd;
                players.push_back(new_player);

                const char* msg = "prosze podac nick <01xxxxx>\n";
                ssize_t sent = send(new_fd, msg, strlen(msg), 0);
                if (sent < (int)strlen(msg)) {
                    perror("send (prośba o nick)");
                    return 1;
                }


            } else {
                // std::cout << "clientfd" << std::endl;
                while (true) {
                    char buffer[1024] = {0};
                    int bytes_received = recv(events[n].data.fd, buffer, sizeof(buffer) - 1, 0);
                    if (bytes_received < 0) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            // nie ma więcej danych
                            break;
                        }
                        perror("recv (wiadomość od klienta)");
                        close(events[n].data.fd);
                        epoll_ctl(efd, EPOLL_CTL_DEL, events[n].data.fd, &ev);
                        fds_to_watch--;
                        break;
                    } else if (bytes_received == 0) {
                        // klient się rozłączył
                        close(events[n].data.fd);
                        epoll_ctl(efd, EPOLL_CTL_DEL, events[n].data.fd, &ev);
                        fds_to_watch--;
                        break;
                    }
                    //write(1, buffer, bytes_received); // do sprawdzania odebranej wiadomości
                    std::string client_msg(buffer, bytes_received);

                    handle_client_message(events[n].data.fd, client_msg);
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

    struct addrinfo* addr_iterator;
    // resolved now points to a linked list of 1 or more struct addrinfos
    for (addr_iterator = resolved; addr_iterator != NULL; addr_iterator = addr_iterator->ai_next) {
        // make a socket:
        sockfd = socket(addr_iterator->ai_family, addr_iterator->ai_socktype, addr_iterator->ai_protocol);
        if (sockfd == -1) {
            perror("socket");
            continue;
        }

        // make the sock non-blocking
        setNonBlocking(sockfd);

        // bind it to the port
        int failBind = bind(sockfd, addr_iterator->ai_addr, addr_iterator->ai_addrlen);
        if (failBind == -1) {
            close(sockfd);
            perror("bind");
            continue;
        }

        break;
    }

    if (addr_iterator == NULL) {
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