#include "Game.hpp"
#include "FileIO.hpp"
#include "Utils.hpp"
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
    setColor(ConsoleColor::Cyan);
    std::cout << "\n=== Survivor Island ===\n";
    resetColor();

    setColor(ConsoleColor::Yellow);
    std::cout << player_.summary() << "\n";
    resetColor();
}

void Game::printMenu() const {
    std::cout << "\nChoose an action:\n";
    auto n = [&](int i) { setColor(ConsoleColor::Cyan); std::cout << " " << i << ") "; resetColor(); };

    n(1); std::cout << "Forage (food)\n";
    n(2); std::cout << "Collect water\n";
    n(3); std::cout << "Gather wood\n";
    n(4); std::cout << "Explore (random event)\n";
    n(5); std::cout << "Rest\n";
    n(6); std::cout << "Build shelter (cost: 5 wood)\n";
    n(7); std::cout << "Eat (-1 food +10 energy)\n";
    n(8); std::cout << "Drink (-1 water +10 energy)\n";
    n(9); std::cout << "Save & Quit\n";
    std::cout << "> ";
}

void Game::doAction(int c) {
    switch (c) {
    case 1: player_.energy -= 10; player_.food += 1; break;
    case 2: player_.energy -= 10; player_.water += 1; break;
    case 3: player_.energy -= 12; player_.wood += 2; break;
    case 4: player_.energy -= 8; applyEvent(); break;
    case 5: player_.energy += player_.hasShelter ? 25 : 15; break;
    case 6:
        if (player_.wood >= 5) {
            player_.wood -= 5; player_.hasShelter = true;
            setColor(ConsoleColor::Green);
            std::cout << "You build a simple shelter.\n";
            resetColor();
        }
        else {
            setColor(ConsoleColor::Red);
            std::cout << "Not enough wood.\n";
            resetColor();
        }
        break;
    case 7:
        if (player_.food > 0) {
            player_.food--; player_.energy += 10; player_.daysSinceEat = 0;
        }
        else {
            setColor(ConsoleColor::Red); std::cout << "You have no food.\n"; resetColor();
        }
        break;
    case 8:
        if (player_.water > 0) {
            player_.water--; player_.energy += 10; player_.daysSinceDrink = 0;
        }
        else {
            setColor(ConsoleColor::Red); std::cout << "You have no water.\n"; resetColor();
        }
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

    int score = picked->dEnergy + picked->dFood + picked->dWater + picked->dWood + picked->dHealth;
    ConsoleColor col = ConsoleColor::Gray;
    if (score > 0)      col = ConsoleColor::Green;
    else if (score < 0) col = ConsoleColor::Red;

    setColor(col);
    std::cout << "\n>> " << picked->description << "\n";
    resetColor();

    player_.energy += picked->dEnergy;
    player_.food += picked->dFood;
    player_.water += picked->dWater;
    player_.wood += picked->dWood;
    player_.health += picked->dHealth;
    player_.clamp();
}

void Game::nextDayTick() {
    player_.day++;
    player_.daysSinceEat++;
    player_.daysSinceDrink++;

    if (player_.daysSinceEat == 2) {
        setColor(ConsoleColor::Yellow);
        std::cout << "Your stomach growls. Consider eating soon.\n";
        resetColor();
    }
    if (player_.daysSinceDrink == 2) {
        setColor(ConsoleColor::Yellow);
        std::cout << "You feel parched. Consider drinking soon.\n";
        resetColor();
    }

    // Food check
    if (player_.daysSinceEat >= 3) {
        if (player_.food > 0) {
            player_.food--;
            player_.daysSinceEat = 0;
            setColor(ConsoleColor::Green);
            std::cout << "You eat some food to keep going.\n";
            resetColor();
        }
        else {
            int hpLoss = (player_.daysSinceEat >= 4) ? 12 : 6;
            int enLoss = (player_.daysSinceEat >= 4) ? 18 : 10;
            player_.health -= hpLoss;
            player_.energy -= enLoss;
            setColor(ConsoleColor::Red);
            std::cout << "You are starving (" << player_.daysSinceEat
                << " days without food). HP-" << hpLoss << " EN-" << enLoss << "\n";
            resetColor();
        }
    }

    // Water check
    if (player_.daysSinceDrink >= 3) {
        if (player_.water > 0) {
            player_.water--;
            player_.daysSinceDrink = 0;
            setColor(ConsoleColor::Green);
            std::cout << "You drink water and feel better.\n";
            resetColor();
        }
        else {
            int hpLoss = (player_.daysSinceDrink >= 4) ? 25 : 12;
            int enLoss = (player_.daysSinceDrink >= 4) ? 25 : 15;
            player_.health -= hpLoss;
            player_.energy -= enLoss;
            setColor(ConsoleColor::Red);
            std::cout << "You are dehydrated (" << player_.daysSinceDrink
                << " days without water). HP-" << hpLoss << " EN-" << enLoss << "\n";
            resetColor();
        }
    }

    if (player_.hasShelter) { player_.energy += 5; }
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
        if (player_.isDead()) {
            std::cout << "\nYou collapse. The island claims another soul.\n";
            break;
        }

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
