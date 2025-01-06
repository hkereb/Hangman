#ifndef HANDLE_CLIENT_MESSAGE_H
#define HANDLE_CLIENT_MESSAGE_H

#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include <ctime>
#include <list>

#include "network.h"
#include "helpers.h"
#include "Settings.h"
#include "Player.h"
#include "Game.h"
#include "Lobby.h"

void handleClientMessage(int clientFd, std::string msg);

#endif