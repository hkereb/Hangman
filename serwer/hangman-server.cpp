#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <netdb.h>
#include <string.h>
#include <sys/sendfile.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <errno.h>
#include <iostream>
#include <list>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <map>

#define MAXEPOLLSIZE 1000
#define BACKLOG 200 // how many pending connections queue will hold
#define MAXPLAYERS 4
struct Player {
    int sockfd;
    std::string nick;
};
std::vector<Player> players;

struct Lobby {
    std::string name;
    std::string password;
    int count_players;
    std::vector<Player> players;

    int difficulty = 1;
    int rounds_amount = 5;
    int round_duration = 60;
};

std::list<Lobby> gameLobbies;
int lobbyCount = 0;

std::vector<std::string> client_nicks;

int setNonBlocking(int sockfd);
int startListening();

// funkcja do mapowania ustawien w formie difficulty=3&round_duration=120 itd.
std::map<std::string, int> parseSettings(const std::string& settings) {
    std::map<std::string, int> parsedSettings;
    std::istringstream ss(settings);
    std::string pair;

    while (std::getline(ss, pair, '&')) {
        size_t pos = pair.find('=');
        if (pos != std::string::npos) {
            std::string key = pair.substr(0, pos);
            int value = std::stoi(pair.substr(pos + 1));
            parsedSettings[key] = value;
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

        auto it = std::find_if(players.begin(), players.end(), [client_fd](const Player& player) {
            return player.sockfd == client_fd;
        });

        if (it != players.end()) {
            it->nick = nick;
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
        auto it = std::find_if(gameLobbies.begin(), gameLobbies.end(), [&lobby_name](const Lobby& lobby) {
            return lobby.name == lobby_name;
        });

        if (it != gameLobbies.end()) {
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
        auto it = std::find_if(gameLobbies.begin(), gameLobbies.end(), [&room_name](const Lobby& lobby) {
            return lobby.name == room_name;
        });

        if (it == gameLobbies.end()) {
            sendToClient(client_fd, "03", "0\nroomNotExists\n");  // Pokój nie istnieje
            return;
        }

        if (it->count_players > MAXPLAYERS) {
            sendToClient(client_fd, "03", "0\nroomIsFull\n");  // Pokój jest pełny
            return;
        }
        // Znajdź gracza
        auto player_it = std::find_if(players.begin(), players.end(), [client_fd](const Player& player) {
            return player.sockfd == client_fd;
        });
        
        if (player_it != players.end()) {
            it->players.push_back(*player_it);  // Dodaj gracza do pokoju
            it->count_players++;
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
            if (parsedSettings.find("rounds_amount") != parsedSettings.end()) {
                lobby_it->rounds_amount = parsedSettings["rounds_amount"];
            }
            if (parsedSettings.find("round_duration") != parsedSettings.end()) {
                lobby_it->round_duration = parsedSettings["round_duration"];
            }

            // wypisanie ustawien
            std::cout << "Updated settings for lobby " << lobby_it->name << ":\n";
            std::cout << "  Difficulty: " << lobby_it->difficulty << "\n";
            std::cout << "  Rounds: " << lobby_it->rounds_amount << "\n";
            std::cout << "  Duration: " << lobby_it->round_duration << "\n";

            sendToClient(client_fd, "04", "1\nsettingsAssignmentSuccessful\n");  // Success
        } else {
            sendToClient(client_fd, "04", "0\nsettingsAssignmentFailed\n");  // Failed
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