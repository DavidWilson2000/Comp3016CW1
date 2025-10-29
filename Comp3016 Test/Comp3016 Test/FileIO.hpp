#pragma once
#include "Event.hpp"
#include "Player.hpp"
#include <vector>
#include <string>

namespace FileIO {
    Player loadPlayerInit(const std::string& path);
    std::vector<Event> loadEventsTSV(const std::string& path);
    void saveGame(const std::string& path, const Player& p);
    Player loadSave(const std::string& path);
}
