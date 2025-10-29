#include "Game.hpp"
#include "FileIO.hpp"
#include "Utils.hpp"
#include <iostream>
#include <chrono>
#include <conio.h>   // for _kbhit() on Windows


// --------------------- ctor / dtor ---------------------

Game::Game(const std::string& dataDir)
    : rng_(static_cast<unsigned>(
        std::chrono::high_resolution_clock::now().time_since_epoch().count())),
    dataDir_(dataDir)
{
    // Load data
    try {
        player_ = FileIO::loadPlayerInit(dataDir_ + "/player_init.txt");
        events_ = FileIO::loadEventsTSV(dataDir_ + "/events.tsv");
    }
    catch (const std::exception& e) {
        std::cerr << "[FATAL] " << e.what() << "\n";
        std::exit(1);
    }

    // SDL setup (HUD window)
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
        std::exit(1);
    }
    window_ = SDL_CreateWindow("Survivor Island HUD", 800, 500, SDL_WINDOW_RESIZABLE);
    renderer_ = SDL_CreateRenderer(window_, nullptr);
    if (!window_ || !renderer_) {
        std::cerr << "SDL create failed: " << SDL_GetError() << "\n";
        std::exit(1);
    }

    // First render so the HUD is visible immediately
    renderHUD();
}

Game::~Game() {
    if (renderer_) SDL_DestroyRenderer(renderer_);
    if (window_)   SDL_DestroyWindow(window_);
    SDL_Quit();
}

// --------------------- UI printing (console) ---------------------

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

// --------------------- core actions ---------------------

void Game::doAction(int c) {
    switch (c) {
    case 1: player_.energy -= 10; player_.food += 1; break;    // forage
    case 2: player_.energy -= 10; player_.water += 1; break;    // collect water
    case 3: player_.energy -= 12; player_.wood += 2; break;    // gather wood
    case 4: player_.energy -= 8;  applyEvent();      break;     // explore
    case 5: player_.energy += player_.hasShelter ? 25 : 15; break; // rest
    case 6: // build shelter
        if (player_.wood >= 5) {
            player_.wood -= 5; player_.hasShelter = true;
            setColor(ConsoleColor::Green); std::cout << "You build a simple shelter.\n"; resetColor();
        }
        else {
            setColor(ConsoleColor::Red); std::cout << "Not enough wood.\n"; resetColor();
        }
        break;
    case 7: // eat
        if (player_.food > 0) {
            player_.food--; player_.energy += 10; player_.daysSinceEat = 0;
        }
        else {
            setColor(ConsoleColor::Red); std::cout << "You have no food.\n"; resetColor();
        }
        break;
    case 8: // drink
        if (player_.water > 0) {
            player_.water--; player_.energy += 10; player_.daysSinceDrink = 0;
        }
        else {
            setColor(ConsoleColor::Red); std::cout << "You have no water.\n"; resetColor();
        }
        break;
    case 9: // save & quit
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

// --------------------- events ---------------------

void Game::applyEvent() {
    // weighted pick
    int totalW = 0;
    for (auto& e : events_) totalW += e.weight;
    std::uniform_int_distribution<int> dist(1, totalW);
    int r = dist(rng_);

    const Event* picked = nullptr;
    int cum = 0;
    for (auto& e : events_) { cum += e.weight; if (r <= cum) { picked = &e; break; } }
    if (!picked) return;

    // choose color by net effect
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

// --------------------- daily tick (hunger/thirst) ---------------------

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

    // Food check (auto-eat every 3 days if available)
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

    // Water check (dehydration harsher)
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

    // Refresh the HUD each day
    renderHUD();
}
int Game::readMenuChoice() {
    std::string line;

    for (;;) {
        // Keep the SDL window responsive
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_EVENT_QUIT) {
                std::cout << "\nWindow closed. Exiting.\n";
                std::exit(0);
            }
        }

        // If there's keyboard input waiting in the console, read it
        if (_kbhit()) {
            std::getline(std::cin, line);
            if (line.empty()) continue;
            try {
                return std::stoi(line);
            }
            catch (...) {
                std::cout << "Please enter a number.\n> ";
                std::cout.flush();
            }
        }

        // Keep the HUD repainting smoothly
        renderHUD();
        SDL_Delay(16); // ~60 FPS idle
    }
}

// --------------------- main loop ---------------------

void Game::run() {
    std::cout << "Load previous game? (y/n): ";
    char yn; std::cin >> yn;
    if (yn == 'y' || yn == 'Y') {
        try { player_ = FileIO::loadSave(dataDir_ + "/save.txt"); std::cout << "Save loaded.\n"; }
        catch (...) { std::cout << "No valid save found; starting new.\n"; }
        renderHUD();
    }

    while (true) {
        // let user close SDL window if they want
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_EVENT_QUIT) {
                std::cout << "\nWindow closed. Exiting.\n";
                return;
            }
        }

        printHeader();
        if (player_.isDead()) {
            std::cout << "\nYou collapse. The island claims another soul.\n";
            break;
        }

        printMenu();
        std::cout.flush();
        int choice = readMenuChoice();


        doAction(choice);
        nextDayTick();
    }
}

// --------------------- HUD rendering (SDL) ---------------------

void Game::renderHUD() const {
    // --- Static state for animations ---
    static float prevFood = player_.food;
    static float prevWater = player_.water;
    static float prevWood = player_.wood;
    static float animTimer = 0.0f;

    // update animation timer
    animTimer += 0.05f;

    // check for value changes (pulse trigger)
    bool pulseFood = (player_.food != static_cast<int>(prevFood));
    bool pulseWater = (player_.water != static_cast<int>(prevWater));
    bool pulseWood = (player_.wood != static_cast<int>(prevWood));

    if (pulseFood)  prevFood = player_.food;
    if (pulseWater) prevWater = player_.water;
    if (pulseWood)  prevWood = player_.wood;

    // clear bg
    SDL_SetRenderDrawColor(renderer_, 25, 30, 60, 255);
    SDL_RenderClear(renderer_);

    int w, h;
    SDL_GetWindowSize(window_, &w, &h);

    // ---- HEALTH BAR (top) ----
    float barX = 20.0f, barY = 20.0f;
    float barWidth = static_cast<float>(w - 40);
    if (barWidth < 100.0f) barWidth = 100.0f;
    float barHeight = 30.0f;
    float filled = barWidth * (player_.health / 100.0f);

    // background
    SDL_FRect bg{ barX, barY, barWidth, barHeight };
    SDL_SetRenderDrawColor(renderer_, 60, 60, 60, 255);
    SDL_RenderFillRect(renderer_, &bg);

    // fill (red -> green)
    SDL_SetRenderDrawColor(renderer_,
        (Uint8)(255 - (int)(player_.health * 2.55f)),
        (Uint8)((int)(player_.health * 2.55f)),
        0, 255);
    SDL_FRect fg{ barX, barY, filled, barHeight };
    SDL_RenderFillRect(renderer_, &fg);

    // ---- RESOURCE BOXES ----
    float boxSize = 80.0f;
    float padding = 20.0f;
    float startY = 80.0f;
    float startX = 20.0f;

    struct Item {
        const char* name;
        int value;
        SDL_Color color;
        bool pulse;
    };
    Item items[] = {
        {"Food",  player_.food,  {200,200, 50,255}, pulseFood},
        {"Water", player_.water, { 80,180,255,255}, pulseWater},
        {"Wood",  player_.wood,  {139, 69, 19,255}, pulseWood}
    };

    // render boxes and icons
    for (int i = 0; i < 3; ++i) {
        SDL_FRect box{ startX + i * (boxSize + padding), startY, boxSize, boxSize };

        // outer box
        SDL_SetRenderDrawColor(renderer_, 40, 40, 40, 255);
        SDL_RenderFillRect(renderer_, &box);

        // slight pulsing brightness when resource changes
        float pulse = 1.0f + (items[i].pulse ? 0.2f * sin(animTimer * 8.0f) : 0.0f);
        Uint8 r = std::min(255, (int)(items[i].color.r * pulse));
        Uint8 g = std::min(255, (int)(items[i].color.g * pulse));
        Uint8 b = std::min(255, (int)(items[i].color.b * pulse));

        // colored inner block
        SDL_SetRenderDrawColor(renderer_, r, g, b, 255);
        SDL_FRect inner{ box.x + 8.0f, box.y + 8.0f, box.w - 16.0f, box.h - 16.0f };
        SDL_RenderFillRect(renderer_, &inner);

        // white frame
        SDL_SetRenderDrawColor(renderer_, 255, 255, 255, 255);
        SDL_RenderRect(renderer_, &box);

        // --- Simple icons ---
        if (i == 0) {
            // food: oval
            SDL_SetRenderDrawColor(renderer_, 255, 220, 100, 255);
            for (int y = -15; y < 15; ++y) {
                for (int x = -20; x < 20; ++x) {
                    float fx = static_cast<float>(x);
                    float fy = static_cast<float>(y);
                    if ((fx * fx) / 400.0f + (fy * fy) / 225.0f <= 1.0f) {
                        SDL_RenderPoint(renderer_,
                            box.x + box.w * 0.5f + fx,
                            box.y + box.h * 0.5f + fy);
                    }
                }
            }
        }
        else if (i == 1) {
            // water: droplet
            SDL_SetRenderDrawColor(renderer_, 100, 180, 255, 255);
            for (int y = 0; y < 30; ++y) {
                for (int x = -15; x < 15; ++x) {
                    float fx = static_cast<float>(x);
                    float fy = static_cast<float>(y);
                    if (fx * fx + (fy - 10.0f) * (fy - 10.0f) / 2.0f < 200.0f) {
                        SDL_RenderPoint(renderer_,
                            box.x + box.w * 0.5f + fx,
                            box.y + 15.0f + fy);
                    }
                }
            }
        }
        else {
            // wood: log rectangle
            SDL_SetRenderDrawColor(renderer_, 139, 69, 19, 255);
            SDL_FRect log{ box.x + 15.0f, box.y + 25.0f, 50.0f, 20.0f };
            SDL_RenderFillRect(renderer_, &log);
        }

        // --- resource value bars ---
        int value = items[i].value;
        int maxUnits = 10;
        float unitW = 6.0f;
        float unitH = 10.0f;
        float gap = 2.0f;
        float totalW = (unitW + gap) * maxUnits;

        float startXUnits = box.x + box.w / 2.0f - totalW / 2.0f;
        float startYUnits = box.y + box.h + 10.0f;

        for (int u = 0; u < maxUnits; ++u) {
            SDL_FRect unit = { startXUnits + u * (unitW + gap), startYUnits, unitW, unitH };

            if (u < value) {
                // brighter pulse when recently changed
                Uint8 ur = std::min(255, (int)(items[i].color.r * (1.0f + 0.3f * sin(animTimer * 10.0f))));
                Uint8 ug = std::min(255, (int)(items[i].color.g * (1.0f + 0.3f * sin(animTimer * 10.0f))));
                Uint8 ub = std::min(255, (int)(items[i].color.b * (1.0f + 0.3f * sin(animTimer * 10.0f))));
                SDL_SetRenderDrawColor(renderer_, ur, ug, ub, 255);
            }
            else {
                SDL_SetRenderDrawColor(renderer_, 80, 80, 80, 255);  // empty slot
            }

            SDL_RenderFillRect(renderer_, &unit);
        }
    }

    SDL_RenderPresent(renderer_);
}
