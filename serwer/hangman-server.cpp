#include <cstring>
#include <iostream>
#include <list>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <poll.h>
#include <pthread.h>
#include <thread>
#include <vector>

using namespace std;

struct Player {
    int sockfd;
    string nick;
    string room_name;
    int score;
};

struct Lobby {
    string name;
    string password;
    int count;
    std::vector<Player> players;
};

int sockfd;

std::list<Lobby> gameLobbies;
int lobbyCount = 0;

vector<int> client_sockets;

void handle_client(int clientfd) {
    const char* msg = "serwer: witaj na serwerze\n";
    send(clientfd, msg, strlen(msg), 0);
    //close(clientfd);
    std::thread::id thread_id = std::this_thread::get_id();
    std::cout << "serwer: klient działa na wątku o ID: " << thread_id << std::endl;
    while(true) {
        int i = 1;
    }
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

    fail = listen(sockfd, 1);
    if (fail == -1) {
        perror("Error (listen)");
        return 1;
    }

    pollfd pfds[1];
    pfds[0].fd = sockfd;
    pfds[0].events = POLLIN;

    while(true) {
        // poll czekający na zdarzenia
        int poll_count = poll(pfds, 1, -1);
        if (poll_count == -1) {
            perror("Error (poll)");
            break;
        }

        if (pfds[0].revents & POLLIN) {
            sockaddr_in client_addr{};
            socklen_t client_len = sizeof(client_addr);
            int clientfd = accept(sockfd, (sockaddr*)&client_addr, &client_len);
            if (clientfd == -1) {
                perror("Error (accept)");
                continue;
            }

            client_sockets.push_back(clientfd);
            printf("serwer: nowe połączenie\n");

            // nowy wątek per klient
            thread client_thread(handle_client, clientfd);
            client_thread.detach();
        }
    }

    shutdown(sockfd, SHUT_WR);
    close(sockfd);
}