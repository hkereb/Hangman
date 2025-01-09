
#ifndef SENDTOCLIENT_H
#define SENDTOCLIENT_H

#include <string>
#include <sys/socket.h>
#include <iostream>

void sendToClient(int clientFd, const std::string& commandNumber, const std::string& body);

#endif 