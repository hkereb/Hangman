#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <sys/socket.h>
#include <string.h>
#include <string>
#include <sstream>
#include <iomanip>
#include "network.h"
#include "Settings.h"
#include "Player.h"
#include "Lobby.h"
#include "sendToClient.h"

void sendLobbiesToClients(std::vector<std::string> lobbyNames, int clientFd = -1);
void sendPlayersToClients(const Lobby* lobby, int ignoreFd = -1);
void sendWordAndPointsToClients(const Lobby* lobby, const Player* playerWhoGuessed);
void sendLivesToClients(const Lobby* lobby, const Player* playerWhoMissed);
void sendStartToClients(const Lobby* lobby);
void sendEndToClients(const Lobby* lobby);

void isStartAllowed(const Lobby* lobby);
Settings parseSettings(const std::string& msg);
std::string messageSubstring(std::string msg);
int getLobbyCount();

void removeFromLobby(int clientFd);
void removeEmptyLobbies();
