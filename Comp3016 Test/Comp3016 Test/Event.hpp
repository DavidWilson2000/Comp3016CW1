#pragma once
#include <string>

struct Event {
    std::string type;
    std::string description;
    int dEnergy{ 0 }, dFood{ 0 }, dWater{ 0 }, dWood{ 0 }, dHealth{ 0 };
    int weight{ 1 };
};
