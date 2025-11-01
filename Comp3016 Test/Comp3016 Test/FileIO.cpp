#include "FileIO.hpp"
#include "Utils.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>   
#include <algorithm>

static int toInt(const std::string& s) {
    try { return std::stoi(s); }
    catch (...) { throw std::runtime_error("Invalid integer: " + s); }
}

static int parseDelta(const std::string& s) {
    try { return std::stoi(s); }
    catch (...) { throw std::runtime_error("Bad delta: " + s); }
}

Player FileIO::loadPlayerInit(const std::string& path) {
    std::ifstream in(path);
    if (!in) throw std::runtime_error("Could not open " + path);

    Player p;
    std::string line;
    while (std::getline(in, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;
        auto eq = line.find('=');
        if (eq == std::string::npos) continue;
        auto k = trim(line.substr(0, eq));
        auto v = trim(line.substr(eq + 1));

        if (k == "health") p.health = toInt(v);
        else if (k == "energy") p.energy = toInt(v);
        else if (k == "food") p.food = toInt(v);
        else if (k == "water") p.water = toInt(v);
        else if (k == "wood") p.wood = toInt(v);
        else if (k == "day") p.day = toInt(v);
        else if (k == "has_shelter") p.hasShelter = (toInt(v) != 0);
        else if (k == "days_since_eat") p.daysSinceEat = toInt(v);
        else if (k == "days_since_drink") p.daysSinceDrink = toInt(v);
    }
    p.clamp();
    return p;
}

std::vector<Event> FileIO::loadEventsTSV(const std::string& path) {
    std::ifstream in(path);
    if (!in)
        throw std::runtime_error("Could not open " + path);

    std::vector<Event> out;
    std::string line;
    while (std::getline(in, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#')
            continue;

        std::stringstream ss(line);
        std::string col;
        std::vector<std::string> cols;
        while (std::getline(ss, col, '\t'))
            cols.push_back(trim(col));

        // Basic validation: must have at least the core 8 columns
        if (cols.size() < 8)
            continue;

        Event e;
        e.type = cols[0];
        e.description = cols[1];
        e.dEnergy = parseDelta(cols[2]);
        e.dFood = parseDelta(cols[3]);
        e.dWater = parseDelta(cols[4]);
        e.dWood = parseDelta(cols[5]);
        e.dHealth = parseDelta(cols[6]);
        e.weight = toInt(cols[7]);
        if (e.weight < 1)
            e.weight = 1;

        // Optional chaining columns (safe if not present)
        if (cols.size() > 8) e.key = cols[8];
        if (cols.size() > 9) e.chainNext = cols[9];

        out.push_back(e);
    }

    if (out.empty())
        throw std::runtime_error("No events loaded from " + path);

    // Debug summary (optional)
    std::cout << "[DEBUG] Loaded " << out.size() << " events from " << path << "\n";
    size_t chained = std::count_if(out.begin(), out.end(),
        [](const Event& e) { return !e.chainNext.empty(); });
    if (chained > 0)
        std::cout << "         (" << chained << " have chain links)\n";

    return out;
}


void FileIO::saveGame(const std::string& path, const Player& p) {
    std::ofstream out(path);
    if (!out) throw std::runtime_error("Could not write " + path);
    out << "health=" << p.health << "\n";
    out << "energy=" << p.energy << "\n";
    out << "food=" << p.food << "\n";
    out << "water=" << p.water << "\n";
    out << "wood=" << p.wood << "\n";
    out << "day=" << p.day << "\n";
    out << "has_shelter=" << (p.hasShelter ? 1 : 0) << "\n";
    out << "days_since_eat=" << p.daysSinceEat << "\n";
    out << "days_since_drink=" << p.daysSinceDrink << "\n";
}

Player FileIO::loadSave(const std::string& path) {
    return loadPlayerInit(path);
}
