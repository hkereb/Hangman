#ifndef NETWORK_UTILS_H
#define NETWORK_UTILS_H

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#include "constants.h"

int setNonBlocking(int sockfd);
int startListening();

#endif // NETWORK_UTILS_H