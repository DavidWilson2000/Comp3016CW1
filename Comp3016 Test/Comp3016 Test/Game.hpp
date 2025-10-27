#pragma once
#include "Player.hpp"
#include "Event.hpp"
#include <vector>
#include <random>
#include <string>

class Game {
public:
    Game(const std::string& dataDir);
    void run();

private:
    Player player_;
    std::vector<Event> events_;
    std::mt19937 rng_;
    std::string dataDir_;

    void printHeader() const;
    void printMenu() const;
    void doAction(int choice);
    void applyEvent();
    void nextDayTick();
};
