#include <cstring>
#include <iostream>
#include <list>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <thread>
#include <vector>
#include <string>

using namespace std;

struct Player {
    int sockfd;
    string nick;
    string room_name;
    int score;
};
std::vector<Player> players;

struct Lobby {
    string name;
    string password;
    int count;
    std::vector<Player> players;
};

int sockfd;

std::list<Lobby> gameLobbies;
int lobbyCount = 0;

void handle_client(int clientfd) {
    Player new_player;
    new_player.sockfd = clientfd;
    players.push_back(new_player);
}

int main(int argc, char** argv) {
    if (argc != 2) {
        perror("Error (arguments) <port>");
        return 1;
    }

    char* endp;
    long port = strtol(argv[1], &endp, 10);

    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd == -1) {
        perror("Error (socket)");
        return 1;
    }

    const int one = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

    sockaddr_in server_addr {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(uint16_t(port));
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int fail = bind(sockfd, (sockaddr*)&server_addr, sizeof(server_addr));
    if (fail == -1) {
        perror("Error (bind)");
        return 1;
    }

    fail = listen(sockfd, 10);
    if (fail == -1) {
        perror("Error (listen)");
        return 1;
    }

    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        perror("Error (epoll_create1)");
        return 1;
    }

    epoll_event event{};
    event.data.fd = sockfd;
    event.events = EPOLLIN;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sockfd, &event) == -1) {
        perror("Error (epoll_ctl)");
        return 1;
    }

    const int MAX_EVENTS = 10;
    epoll_event events[MAX_EVENTS];

    while (true) {
        int event_count = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (event_count == -1) {
            perror("Error (epoll_wait)");
            break;
        }

        for (int i = 0; i < event_count; ++i) {
            if (events[i].data.fd == sockfd) {
                int clientfd = accept(sockfd, nullptr, nullptr);
                if (clientfd == -1) {
                    perror("Error (accept)");
                    continue;
                }

                cout << "serwer: nowe połączenie";

                epoll_event client_event{};
                client_event.data.fd = clientfd;
                client_event.events = EPOLLIN;

                // dodanie klienta do epoll
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, clientfd, &client_event) == -1) {
                    perror("Error (epoll_ctl - client)");
                    close(clientfd);
                    continue;
                }

                handle_client(clientfd);

                // Usuwamy klienta z epolla po obsłużeniu
                if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, clientfd, nullptr) == -1) {
                    perror("Error (epoll_ctl - remove client)");
                }
            }
        }
    }

    close(epoll_fd);
    shutdown(sockfd, SHUT_WR);
    close(sockfd);

    return 0;
}