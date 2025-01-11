#include "handle-client-message.h"
#include "network.h"
#include "helpers.h"

#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <sys/epoll.h>

#include <iostream>
#include <list>
#include <unordered_map>

std::vector<std::shared_ptr<Player>> players;
std::vector<std::string> playersNicknames;

std::vector<std::shared_ptr<Lobby>> lobbies;
std::vector<std::string> lobbyNames;

int main() {
    int sockfd = startListening();

    int efd = epoll_create1(0);

    epoll_event ev{};
    ev.events = EPOLLIN;
    ev.data.fd = sockfd;

    if (epoll_ctl(efd, EPOLL_CTL_ADD, sockfd, &ev) < 0) {
        perror("epoll_ctl");
        return -1;
    }
    else {
        printf("success insert listening socket into epoll.\n");
    }

    int fdsToWatch = 1;
    int eventsCapacity = fdsToWatch;
    struct epoll_event *events;
    events = (struct epoll_event *)malloc(sizeof(struct epoll_event) * eventsCapacity);

    std::unordered_map<int, std::string> clientBuffers;

    while (1) {  // loop for accepting incoming connections
        int ready = epoll_wait(efd, events, eventsCapacity, -1);
        if (ready == -1) {
            perror("epoll_wait");
            break;
        }

        if (fdsToWatch > eventsCapacity) { // resizing events structure size
            eventsCapacity = fdsToWatch;
            events = (struct epoll_event*)realloc(events, sizeof(struct epoll_event) * eventsCapacity);
            std::cout << "events structure resized" << std::endl;
        }

        for (int n = 0; n < ready; ++n) {
            if (events[n].data.fd == sockfd) {
                sockaddr_in clientAddr{};
                socklen_t addrSize = sizeof(clientAddr);
                const int newFd = accept(events[n].data.fd, reinterpret_cast<struct sockaddr *>(&clientAddr), &addrSize);
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

                auto newPlayer = std::make_shared<Player>();
                newPlayer->sockfd = newFd;
                players.push_back(newPlayer);


                sendToClient(newFd, "69", "hello!");
            }
            else {
                if (events[n].events & (EPOLLERR | EPOLLHUP)) { // epoll errors and hang ups
                    std::cout << "Error or hang-up detected on socket: " <<  events[n].data.fd << std::endl;

                    removeFromLobby(events[n].data.fd);

                    auto playerIt = std::find_if(players.begin(), players.end(), [n, events](const std::shared_ptr<Player>& player) {
                        return player->sockfd == events[n].data.fd;
                    });

                    if (playerIt != players.end()) {
                        auto nicknameIt = std::find(playersNicknames.begin(), playersNicknames.end(), (*playerIt)->nick);
                        if (nicknameIt != playersNicknames.end()) {
                            playersNicknames.erase(nicknameIt);
                        }
                        std::cout << "Player with nickname " << (*playerIt)->nick << " removed from global player list.\n";
                        players.erase(playerIt);
                    }
                    sendLobbiesToClients(lobbyNames);

                    // clean up resources
                    close(events[n].data.fd);
                    epoll_ctl(efd, EPOLL_CTL_DEL, events[n].data.fd, &ev);
                    clientBuffers.erase(events[n].data.fd);  // clean up client buffers
                    fdsToWatch--;

                    continue;
                }
                while (true) {
                    char buffer[1024] = {0};
                    int bytesReceived = recv(events[n].data.fd, buffer, sizeof(buffer) - 1, 0);
                    if (bytesReceived <= 0) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            break;
                        }
                        perror("client got disconnected");

                        removeFromLobby(events[n].data.fd);
                        
                        // remove player from global list
                        auto playerIt = std::find_if(players.begin(), players.end(), [n](const std::shared_ptr<Player>& player) {
                            return player->sockfd == n;
                        });

                        if (playerIt != players.end()) {
                            auto nicknameIt = std::find(playersNicknames.begin(), playersNicknames.end(), (*playerIt)->nick);
                            if (nicknameIt != playersNicknames.end()) {
                                playersNicknames.erase(nicknameIt);
                            }
                            std::cout << "Player with nickname " << (*playerIt)->nick << " removed from global player list.\n";
                            players.erase(playerIt);
                        }
                        sendLobbiesToClients(lobbyNames);

                        // clean up resources
                        close(events[n].data.fd);
                        epoll_ctl(efd, EPOLL_CTL_DEL, events[n].data.fd, &ev);
                        clientBuffers.erase(events[n].data.fd);  // clean up client buffers
                        fdsToWatch--;
                        break;
                    }
                    clientBuffers[events[n].data.fd] += std::string(buffer, bytesReceived);

                    std::string& clientBuffer = clientBuffers[events[n].data.fd];
                    size_t pos;
                    while ((pos = clientBuffer.find('\n')) != std::string::npos) {
                        std::string clientMessage = clientBuffer.substr(0, pos);
                        clientBuffer.erase(0, pos + 1);

                        std::string outputMessage = "klient: " + clientMessage + "\n";
                        write(1, outputMessage.c_str(), outputMessage.size());

                        handleClientMessage(events[n].data.fd, clientMessage);
                    }
                }
            }
        }
    }

    free(events);
    close(sockfd);
    return 0;
}