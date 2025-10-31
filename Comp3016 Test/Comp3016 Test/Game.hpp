#pragma once
#include <string>
#include <vector>
#include <random>
#include <SDL3/SDL.h>
#include "Player.hpp"
#include "Event.hpp"
#include <deque>

// ---- Global enums/types ----
enum class DayPhase { Morning, Day, Evening, Night };
enum class Weather { Clear, Rain, Storm };

// Visible to free functions
struct WorldState {
    DayPhase phase = DayPhase::Morning;
    Weather  weather = Weather::Clear;
    bool     hasCampfire = false;
    bool     hasCollector = false;
    float    lightningFlash = 0.0f; // 0..1
};

struct Director {
    float tension = 0.5f;          // 0..1 (low = kinder, high = harsher)
    int   goodStreak = 0;          // consecutive positive events
    int   badStreak = 0;          // consecutive negative events
    std::string lastType;          // last applied event type (to avoid repeats)

    void  onDayStart(const Player& p, const WorldState& w);
    void  onEventApplied(const Event& e);
    float weatherBias() const;                     // -1..+1: <0 favors clear, >0 favors storm
    float weightMultiplierForType(const std::string& type) const; // scales event weights
};



class Game {
public:
    Game(const std::string& dataDir);
    ~Game();
    void run();

    bool debug_{ false };

private:
    // SDL
    SDL_Window* window_{ nullptr };
    SDL_Renderer* renderer_{ nullptr };


    Director               director_;  

    // Core
    Player                 player_;
    std::vector<Event>     events_;
    std::mt19937           rng_;
    std::string            dataDir_;
    WorldState             world_;

    // Methods
    int   readMenuChoice();
    void  printHeader() const;
    void  printMenu() const;
    void  doAction(int choice);
    void  applyEvent();
    void  nextDayTick();

    void  advanceWeather();
    float weatherWeightMul(const std::string& type) const;
    void  renderHUD();                       // NOTE: non-const
    void  renderDebugPanel(int w, int h) const;
};
