#pragma once
#include <string>
#include <vector>
#include <random>
#include <SDL3/SDL.h>
#include "Player.hpp"
#include "Event.hpp"

class Game {
public:
    Game(const std::string& dataDir);
    ~Game();
    void run();

    // Weather + debug
    enum class Weather { Clear, Rain, Storm };
    Weather weather_{ Weather::Clear };
    bool debug_{ false };

private:
    // SDL
    SDL_Window* window_{ nullptr };
    SDL_Renderer* renderer_{ nullptr };

    // Core
    Player player_;
    std::vector<Event> events_;
    std::mt19937 rng_;
    std::string dataDir_;


    // Methods
    int readMenuChoice();
    void printHeader() const;
    void printMenu() const;
    void doAction(int choice);
    void applyEvent();
    void nextDayTick();

    void advanceWeather();
    float weatherWeightMul(const std::string& type) const;
    void renderHUD() const;
    void renderDebugPanel(int w, int h) const;
};
