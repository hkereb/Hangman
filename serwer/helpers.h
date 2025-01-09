#ifndef COMMUNICATION_FUNCTIONS_H
#define COMMUNICATION_FUNCTIONS_H

#include <string>
#include <vector>
#include <iostream>
#include <sys/socket.h>
#include <string.h>
#include "network.h"
#include "Settings.h"
#include "Player.h"
#include "Game.h"
#include "Lobby.h"


void sendToClient(int clientFd, const std::string& commandNumber, const std::string& body);
void sendLobbiesToClients(std::vector<std::string> lobbyNames, int clientFd = -1);
void sendPlayersToClients(const Lobby* lobby, int ignoreFd = -1);
void isStartAllowed(const Lobby* lobby);
Settings parseSettings(std::string msg);
std::string messageSubstring(std::string msg);
void removeFromLobby(int clientFd);
void sendWordAndPointsToClients(const Lobby* lobby, const Player* playerWhoGuessed);
void sendLivesToClients(const Lobby* lobby, const Player* playerWhoMissed);
void sendStartToClients(const Lobby* lobby);

#endif // COMMUNICATION_FUNCTIONS_H
