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

#define MAXEPOLLSIZE 1000
#define BACKLOG 200 // how many pending connections queue will hold

struct Player {
    int sockfd;
    std::string nick;
    std::string room_name;
    int score;
};
std::vector<Player> players;

struct Lobby {
    std::string name;
    std::string password;
    int count_players;
    std::vector<Player> players;
};
std::list<Lobby> gameLobbies;
int lobbyCount = 0;

std::vector<std::string> client_nicks;

int setNonBlocking(int sockfd);
int startListening();

// obsługa klienta na podstawie jego wiadomości
void handle_client_message(int client_fd, std::string msg) {
    if (msg.compare("nick")) {
        client_nicks.push_back(msg);
        for (int i=0; i < client_nicks.size(); i++) {
            std::cout << client_nicks[i];
        }
    }
    else {
        // tutaj inne wiadomości/komendy
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[]) {
    if (argc != 1) {
        fprintf(stderr, "serwer nie wymaga dodatkowych argumentów\n");
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

                printf("server: connection established...\n");
                setNonBlocking(new_fd);
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = new_fd;
                if (epoll_ctl(efd, EPOLL_CTL_ADD, new_fd, &ev) < 0) {
                    printf("Failed to insert socket into epoll.\n");
                }
                fds_to_watch++;

                const char* msg = "prosze podac nick\n";
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
            perror("server: socket");
            continue;
        }

        // make the sock non-blocking
        setNonBlocking(sockfd);

        // bind it to the port
        int failBind = bind(sockfd, addr_iterator->ai_addr, addr_iterator->ai_addrlen);
        if (failBind == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    if (addr_iterator == NULL) {
        fprintf(stderr, "server: failed to bind\n");
        return 2;
    }

    freeaddrinfo(resolved);  // free the linked-list

    // listen for incoming connection
    int failListen = listen(sockfd, BACKLOG);
    if (failListen == -1) {
        perror("listen");
        exit(1);
    }

    printf("server: waiting for connections...\n");

    return sockfd;
}