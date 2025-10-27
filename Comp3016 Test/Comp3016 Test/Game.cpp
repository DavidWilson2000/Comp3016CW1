#include "Game.hpp"
#include "FileIO.hpp"
#include <iostream>
#include <chrono>

Game::Game(const std::string& dataDir)
    : rng_(static_cast<unsigned>(
        std::chrono::high_resolution_clock::now().time_since_epoch().count())),
    dataDir_(dataDir)
{
    try {
        player_ = FileIO::loadPlayerInit(dataDir_ + "/player_init.txt");
        events_ = FileIO::loadEventsTSV(dataDir_ + "/events.tsv");
    }
    catch (const std::exception& e) {
        std::cerr << "[FATAL] " << e.what() << "\n";
        std::exit(1);
    }
}

void Game::printHeader() const {
    std::cout << "\n=== Survivor Island ===\n" << player_.summary() << "\n";
}

void Game::printMenu() const {
    std::cout << "\nChoose an action:\n"
        << " 1) Forage (food)\n"
        << " 2) Collect water\n"
        << " 3) Gather wood\n"
        << " 4) Explore (random event)\n"
        << " 5) Rest\n"
        << " 6) Build shelter (cost: 5 wood)\n"
        << " 7) Eat (-1 food +10 energy)\n"
        << " 8) Drink (-1 water +10 energy)\n"
        << " 9) Save & Quit\n"
        << "> ";
}

void Game::doAction(int c) {
    switch (c) {
    case 1: player_.energy -= 10; player_.food += 1; break;
    case 2: player_.energy -= 10; player_.water += 1; break;
    case 3: player_.energy -= 12; player_.wood += 2; break;
    case 4: player_.energy -= 8; applyEvent(); break;
    case 5: player_.energy += player_.hasShelter ? 25 : 15; break;
    case 6:
        if (player_.wood >= 5) { player_.wood -= 5; player_.hasShelter = true; std::cout << "You build a simple shelter.\n"; }
        else std::cout << "Not enough wood.\n";
        break;
    case 7:
        if (player_.food > 0) { player_.food--; player_.energy += 10; }
        else std::cout << "You have no food.\n";
        break;
    case 8:
        if (player_.water > 0) { player_.water--; player_.energy += 10; }
        else std::cout << "You have no water.\n";
        break;
    case 9:
        try {
            FileIO::saveGame(dataDir_ + "/save.txt", player_);
            std::cout << "Game saved. Goodbye!\n";
        }
        catch (const std::exception& e) {
            std::cerr << "Save failed: " << e.what() << "\n";
        }
        std::exit(0);
    default:
        std::cout << "Invalid choice.\n";
    }
    player_.clamp();
}

void Game::applyEvent() {
    int totalW = 0;
    for (auto& e : events_) totalW += e.weight;
    std::uniform_int_distribution<int> dist(1, totalW);
    int r = dist(rng_);

    const Event* picked = nullptr;
    int cum = 0;
    for (auto& e : events_) {
        cum += e.weight;
        if (r <= cum) { picked = &e; break; }
    }
    if (!picked) return;

    std::cout << "\n>> " << picked->description << "\n";
    player_.energy += picked->dEnergy;
    player_.food += picked->dFood;
    player_.water += picked->dWater;
    player_.wood += picked->dWood;
    player_.health += picked->dHealth;
    player_.clamp();
}

void Game::nextDayTick() {
    if (player_.food == 0) { player_.health -= 5; }
    if (player_.water == 0) { player_.health -= 7; }
    if (player_.hasShelter) { player_.energy += 5; }
    player_.day++;
    player_.clamp();
}

void Game::run() {
    std::cout << "Load previous game? (y/n): ";
    char yn; std::cin >> yn;
    if (yn == 'y' || yn == 'Y') {
        try { player_ = FileIO::loadSave(dataDir_ + "/save.txt"); std::cout << "Save loaded.\n"; }
        catch (...) { std::cout << "No valid save found; starting new.\n"; }
    }

    while (true) {
        printHeader();
        if (player_.isDead()) { std::cout << "\nYou collapse. The island claims another soul.\n"; break; }

        printMenu();
        int choice = 0;
        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            std::cout << "Please enter a number.\n";
            continue;
        }
        doAction(choice);
        nextDayTick();
    }
}
