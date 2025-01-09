#include "sendToClient.h"

void sendToClient(int clientFd, const std::string& commandNumber, const std::string& body) {
    if (commandNumber.size() != 2) {
        std::cerr << "Error: Command number must consist of 2 characters";
        return;
    }

    std::string fullMessage = commandNumber + "\\" + body + "\n";
    ssize_t bytesSent = send(clientFd, fullMessage.c_str(), fullMessage.size(), 0);

    if (bytesSent == -1) {
        std::cerr << "Error: Failed to send message to client with socket " << clientFd << ".\n";
    }
}