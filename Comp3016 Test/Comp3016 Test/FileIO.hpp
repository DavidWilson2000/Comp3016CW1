#pragma once
#include "Event.hpp"
#include "Player.hpp"
#include <vector>
#include <string>

namespace FileIO {
    Player loadPlayerInit(const std::string& path);            // throws std::runtime_error
    std::vector<Event> loadEventsTSV(const std::string& path); // throws std::runtime_error
    void saveGame(const std::string& path, const Player& p);   // throws std::runtime_error
    Player loadSave(const std::string& path);                  // throws std::runtime_error
}
