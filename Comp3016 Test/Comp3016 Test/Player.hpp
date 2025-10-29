#pragma once
#include <string>
#include <cstdio>

struct Player {
    int health{ 100 };
    int energy{ 100 };
    int food{ 0 };
    int water{ 0 };
    int wood{ 0 };
    int day{ 1 };
    bool hasShelter{ false };

    // New hunger/thirst timers
    int daysSinceEat{ 0 };
    int daysSinceDrink{ 0 };

    void clamp() {
        auto clamp01 = [](int& v, int lo, int hi) { if (v < lo) v = lo; if (v > hi) v = hi; };
        clamp01(health, 0, 100);
        clamp01(energy, 0, 100);
        clamp01(food, 0, 999);
        clamp01(water, 0, 999);
        clamp01(wood, 0, 999);
        if (day < 1) day = 1;
    }

    bool isDead() const { return health <= 0 || energy <= 0; }

    std::string summary() const {
        char buf[250];
        std::snprintf(buf, sizeof(buf),
            "Day %d | Health:%d Energy:%d | Food:%d Water:%d Wood:%d | Shelter:%s | Hunger:%dd Thirst:%dd",
            day, health, energy, food, water, wood,
            hasShelter ? "Yes" : "No", daysSinceEat, daysSinceDrink);
        return buf;
    }
};
