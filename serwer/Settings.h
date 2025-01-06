#ifndef SETTINGS_H
#define SETTINGS_H

#include <string>

struct Settings {
    std::string name;
    std::string password;
    int difficulty;
    int roundsAmount;
    int roundDurationSec;
};

#endif // SETTINGS_H