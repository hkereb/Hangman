#include <cstring>
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <poll.h>
#include <pthread.h>

using namespace std;

int main(int argc, char** argv) {
    // args
    if (argc != 2) {
        perror("Error (arguments) <port>");
        return 1;
    }
    char* endp;
    long port = strtol(argv[1], &endp, 10);

    // socket
    int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd == -1) {
        perror("Error (socket)");
        return 1;
    }

    // sock opt
    const int one = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

    // addr
    sockaddr_in server_addr {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(uint16_t(port));
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // bind
    int fail = bind(sockfd, (sockaddr*)&server_addr, sizeof(server_addr));
    if (fail == -1) {
        perror("Error (bind)");
        return 1;
    }

    // listen
    fail = listen(sockfd, 1);
    if (fail == -1) {
        perror("Error (listen)");
        return 1;
    }

    // config poll
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

            printf("nowe połączenie\n");

            const char* msg = "wiadomość od serwera\n";
            send(clientfd, msg, strlen(msg), 0);

            close(clientfd);
        }
    }

    shutdown(sockfd, SHUT_WR);
    close(sockfd);
}