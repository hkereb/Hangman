#include <cstring>
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

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

    while(true) {
        sockaddr_in new_client{};
        socklen_t new_client_size = sizeof(new_client);

        // accept
        int clientfd = accept(sockfd, (sockaddr*)&new_client, &new_client_size);
        if (clientfd == -1) {
            perror("Error (accept)");
            return 1;
        }

        // act
        const char* msg = "To jest wiadomość od serwera.";
        int count = (int)send(clientfd, msg, strlen(msg), 0);
        if (count != (int)strlen(msg)) {
            perror("Error (send)");
            return 1;
        }

        close(clientfd);
    }

    // shutdown
    shutdown(sockfd, SHUT_WR);

    // close
    close(sockfd);
}