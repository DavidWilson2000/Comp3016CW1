#pragma once
#include <string>

struct Event {
    std::string type;
    std::string description;
    int dEnergy{ 0 }, dFood{ 0 }, dWater{ 0 }, dWood{ 0 }, dHealth{ 0 };
    int weight{ 1 };

std::string key;        // unique id for this event (optional but recommended)
std::string chainNext;  // key of the next event, if any ("" = none)
};