#pragma once
#include "Player.hpp"
#include "Event.hpp"
#include <vector>
#include <random>
#include <string>
#include <SDL3/SDL.h>



class Game {
public:
    Game(const std::string& dataDir);
    SDL_Window* window_{ nullptr };
    SDL_Renderer* renderer_{ nullptr };
    void renderHUD() const;
    ~Game();

    void run();

private:
    Player player_;
    std::vector<Event> events_;
    std::mt19937 rng_;
    std::string dataDir_;
    int readMenuChoice();   // pumps SDL events while waiting for input

    void printHeader() const;
    void printMenu() const;
    void doAction(int choice);
    void applyEvent();
    void nextDayTick();
};
