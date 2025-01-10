#include "network.h"

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

    int status = getaddrinfo(nullptr, "1111", &hints, &resolved);
    if (status != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        return 2;
    }

    struct addrinfo* addrIterator;
    // resolved now points to a linked list of 1 or more struct addrinfos
    for (addrIterator = resolved; addrIterator != nullptr; addrIterator = addrIterator->ai_next) {
        // make a socket:
        sockfd = socket(addrIterator->ai_family, addrIterator->ai_socktype, addrIterator->ai_protocol);
        if (sockfd == -1) {
            perror("socket");
            continue;
        }

        int opt = 1;
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            perror("setsockopt");
            close(sockfd);
            continue;
        }

        // make the sock non-blocking
        setNonBlocking(sockfd);

        // bind it to the port
        int failBind = bind(sockfd, addrIterator->ai_addr, addrIterator->ai_addrlen);
        if (failBind == -1) {
            close(sockfd);
            perror("bind");
            continue;
        }

        break;
    }

    if (addrIterator == nullptr) {
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