#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include <ctime>
#include <list>
#include <cctype>

#include "network.h"
#include "helpers.h"
#include "Settings.h"
#include "Player.h"
#include "Game.h"
#include "Lobby.h"
#include "sendToClient.h"

void handleClientMessage(int clientFd, const std::string& msg);
