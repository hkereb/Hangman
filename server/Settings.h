#pragma once

#include <string>

struct Settings {
    std::string name;
    std::string password;
    int difficulty;
    int roundsAmount;
    int roundDurationSec;
};
