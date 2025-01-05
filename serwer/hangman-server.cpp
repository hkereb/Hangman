#include "handle-client-message.h"
#include "network-utils.h"
#include "comunication-functions.h"

#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <sys/epoll.h>

#include <iostream>
#include <list>

std::vector<Player> players;
std::vector<std::string> playersNicknames;

std::list<Lobby> gameLobbies;
int lobbyCount = 0;
std::vector<std::string> lobbyNames;

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