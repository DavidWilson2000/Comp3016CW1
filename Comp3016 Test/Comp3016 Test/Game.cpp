#include "Game.hpp"
#include "Player.hpp"
#include "Event.hpp"
#include "FileIO.hpp"
#include "Utils.hpp"

#include <iostream>
#include <chrono>
#include <conio.h>
#include <cmath>
#include <vector>
#include <string>
#include <random>
#include <string_view>
#include <fstream>   
#include <sstream>   
#include <cctype>   

//  5x7 Font helpers 
namespace ui {

    static const uint8_t FONT5x7[][7] = {
        {0x00,0x00,0x00,0x00,0x00,0x00,0x00}, // space
        {0x1E,0x11,0x13,0x15,0x19,0x11,0x1E}, // 0
        {0x04,0x0C,0x14,0x04,0x04,0x04,0x1F}, // 1
        {0x1E,0x11,0x01,0x06,0x08,0x10,0x1F}, // 2
        {0x1E,0x11,0x01,0x06,0x01,0x11,0x1E}, // 3
        {0x02,0x06,0x0A,0x12,0x1F,0x02,0x02}, // 4
        {0x1F,0x10,0x1E,0x01,0x01,0x11,0x1E}, // 5
        {0x0E,0x10,0x1E,0x11,0x11,0x11,0x0E}, // 6
        {0x1F,0x01,0x02,0x04,0x08,0x08,0x08}, // 7
        {0x0E,0x11,0x11,0x0E,0x11,0x11,0x0E}, // 8
        {0x0E,0x11,0x11,0x0F,0x01,0x01,0x0E}, // 9
        {0x0E,0x11,0x11,0x1F,0x11,0x11,0x11}, // A
        {0x1E,0x11,0x11,0x1E,0x11,0x11,0x1E}, // B
        {0x0E,0x11,0x10,0x10,0x10,0x11,0x0E}, // C
        {0x1C,0x12,0x11,0x11,0x11,0x12,0x1C}, // D
        {0x1F,0x10,0x10,0x1E,0x10,0x10,0x1F}, // E
        {0x1F,0x10,0x10,0x1E,0x10,0x10,0x10}, // F
        {0x0E,0x11,0x10,0x17,0x11,0x11,0x0E}, // G
        {0x11,0x11,0x11,0x1F,0x11,0x11,0x11}, // H
        {0x1F,0x04,0x04,0x04,0x04,0x04,0x1F}, // I
        {0x1F,0x02,0x02,0x02,0x12,0x12,0x0C}, // J
        {0x11,0x12,0x14,0x18,0x14,0x12,0x11}, // K
        {0x10,0x10,0x10,0x10,0x10,0x10,0x1F}, // L
        {0x11,0x1B,0x15,0x11,0x11,0x11,0x11}, // M
        {0x11,0x19,0x15,0x13,0x11,0x11,0x11}, // N
        {0x0E,0x11,0x11,0x11,0x11,0x11,0x0E}, // O
        {0x1E,0x11,0x11,0x1E,0x10,0x10,0x10}, // P
        {0x0E,0x11,0x11,0x11,0x15,0x12,0x0D}, // Q
        {0x1E,0x11,0x11,0x1E,0x14,0x12,0x11}, // R
        {0x0F,0x10,0x10,0x0E,0x01,0x01,0x1E}, // S
        {0x1F,0x04,0x04,0x04,0x04,0x04,0x04}, // T
        {0x11,0x11,0x11,0x11,0x11,0x11,0x0E}, // U
        {0x11,0x11,0x11,0x11,0x0A,0x0A,0x04}, // V
        {0x11,0x11,0x11,0x15,0x15,0x1B,0x11}, // W
        {0x11,0x11,0x0A,0x04,0x0A,0x11,0x11}, // X
        {0x11,0x11,0x0A,0x04,0x04,0x04,0x04}, // Y
        {0x1F,0x01,0x02,0x04,0x08,0x10,0x1F}, // Z
        {0x00,0x04,0x00,0x00,0x00,0x04,0x00}, // :
        {0x01,0x01,0x02,0x04,0x08,0x10,0x10}, // /
        {0x11,0x09,0x02,0x04,0x08,0x12,0x11}, // %
        {0x00,0x00,0x00,0x1F,0x00,0x00,0x00}, // -
        {0x00,0x04,0x04,0x1F,0x04,0x04,0x00}, // +
    };

    static int glyphIndex(char ch) {
        if (ch == ' ') return 0;
        if (ch >= '0' && ch <= '9') return 1 + (ch - '0');
        if (ch >= 'A' && ch <= 'Z') return 11 + (ch - 'A');
        if (ch >= 'a' && ch <= 'z') return 11 + (ch - 'a');
        if (ch == ':') return 37;
        if (ch == '/') return 38;
        if (ch == '%') return 39;
        if (ch == '-') return 40;
        if (ch == '+') return 41;
        return 0;
    }

    static void DrawChar5x7(SDL_Renderer* r, float x, float y, char ch, float scale, SDL_Color c) {
        int gi = glyphIndex(ch);
        const uint8_t* rows = FONT5x7[gi];
        SDL_SetRenderDrawColor(r, c.r, c.g, c.b, c.a);
        for (int ry = 0; ry < 7; ++ry) {
            uint8_t bits = rows[ry];
            for (int rx = 0; rx < 5; ++rx) {
                if (bits & (1u << (4 - rx))) {
                    SDL_FRect pix{ x + rx * scale, y + ry * scale, scale, scale };
                    SDL_RenderFillRect(r, &pix);
                }
            }
        }
    }

    static void DrawText5x7(SDL_Renderer* r, float x, float y, const std::string& s, float scale, SDL_Color c) {
        float cx = x;
        for (char ch : s) {
            if (ch == '\n') { y += 8 * scale; cx = x; continue; }
            DrawChar5x7(r, cx, y, ch, scale, c);
            cx += 6 * scale;
        }
    }

} // namespace ui

//  Helpers / local state 
static std::string stripTags(const std::string& s) {
    std::string out = s;
    size_t pos = 0;
    while ((pos = out.find('{', pos)) != std::string::npos) {
        size_t end = out.find('}', pos + 1);
        if (end == std::string::npos) break;
        out.erase(pos, end - pos + 1);
    }
    auto ltrim = [](std::string& t) {
        size_t i = 0; while (i < t.size() && std::isspace((unsigned char)t[i])) ++i; t.erase(0, i);
        };
    auto rtrim = [](std::string& t) {
        while (!t.empty() && std::isspace((unsigned char)t.back())) t.pop_back();
        };
    ltrim(out); rtrim(out);
    return out;
}

// Build short string to show what happens after a event
static std::string buildDeltaString(const Event& e) {
    std::string d;
    auto add = [&](const char* label, int v) {
        if (v == 0) return;
        if (!d.empty()) d += ' ';
        d += label;
        if (v > 0) d += '+';
        d += std::to_string(v);
        };
    add("EN", e.dEnergy);
    add("FOOD", e.dFood);
    add("WATER", e.dWater);
    add("WOOD", e.dWood);
    add("HP", e.dHealth);
    if (d.empty()) d = "no change";
    return "[" + d + "]";
}

static inline Uint8 clampU8(int v) {
    if (v < 0) v = 0;
    else if (v > 255) v = 255;
    return (Uint8)v;
}

static int   irand(std::mt19937& rng, int a, int b) { std::uniform_int_distribution<int> d(a, b); return d(rng); }
static float frand(std::mt19937& rng, float a, float b) { std::uniform_real_distribution<float> d(a, b); return d(rng); }

static SDL_Color bgForPhase(DayPhase ph) {
    int r = 25, g = 30, b = 60; float mul = 1.0f;
    switch (ph) {
    case DayPhase::Morning: mul = 1.10f; break;
    case DayPhase::Day:     mul = 1.25f; break;
    case DayPhase::Evening: mul = 0.90f; break;
    case DayPhase::Night:   mul = 0.65f; break;
    }
    auto clamp255 = [](int v) { if (v < 0) v = 0; if (v > 255) v = 255; return v; };
    return SDL_Color{
        (Uint8)clamp255((int)(r * mul)),
        (Uint8)clamp255((int)(g * mul)),
        (Uint8)clamp255((int)(b * mul)),
        255
    };
}

//  Win helper 
static void WinAndQuit(const std::string& reason) {
    setColor(ConsoleColor::Green);
    std::cout << "\n=== YOU WIN! ===\n" << reason << "\n";
    resetColor();
    std::cout << "Thanks for playing.\n";
    std::exit(0);
}

//  AI Director 
void Director::onDayStart(const Player& p, const WorldState& w) {
    float hp = p.health / 100.0f;

    float scarcity = 0.0f;
    if (p.food <= 0)  scarcity += 0.12f;
    if (p.water <= 0) scarcity += 0.16f;
    if (!p.hasShelter) scarcity += 0.08f;

    float weatherStress = (w.weather == Weather::Storm) ? 0.10f
        : (w.weather == Weather::Rain) ? 0.03f : -0.02f;

    float change = scarcity + weatherStress - (hp > 0.8f ? 0.05f : 0.0f);
    tension += change;
    if (tension < 0.0f) tension = 0.0f;
    if (tension > 1.0f) tension = 1.0f;
}

void Director::onEventApplied(const Event& e) {
    int delta = e.dEnergy + e.dFood + e.dWater + e.dWood + e.dHealth;
    lastType = e.type;

    if (delta >= 0) { goodStreak++; badStreak = 0; tension -= 0.05f; }
    else { badStreak++;  goodStreak = 0; tension += 0.06f; }

    if (tension < 0.0f) tension = 0.0f;
    if (tension > 1.0f) tension = 1.0f;
}

float Director::weatherBias() const {
    if (tension > 0.70f)  return -0.35f; // ease weather under stress
    if (tension < 0.25f)  return +0.20f; // makes weather worse if too calm
    return 0.0f;
}

float Director::weightMultiplierForType(const std::string& type) const {
    if (tension > 0.65f) {
        if (type == "WATER")     return 1.35f;
        if (type == "FORAGE")    return 1.20f;
        if (type == "WEATHER")   return 0.75f;
        if (type == "ANIMAL")    return 0.90f;
    }
    else if (tension < 0.25f) {
        if (type == "DISCOVERY") return 1.20f;
        if (type == "ANIMAL")    return 1.15f;
    }
    return 1.0f;
}

//  Difficulty curves + chains 
namespace {

    struct DiffCurve {
        float stormBias;
        float discMul, forageMul, waterMul, animalMul, weatherMul;
    };

    static DiffCurve g_easy{ -0.15f, 1.00f, 1.10f, 1.15f, 0.95f, 0.90f };
    static DiffCurve g_normal{ 0.00f, 1.00f, 1.00f, 1.00f, 1.00f, 1.00f };
    static DiffCurve g_hard{ 0.12f, 1.10f, 0.95f, 0.90f, 1.10f, 1.15f };

    static void tryLoadDifficultyTSV(const std::string& path) {
        std::ifstream f(path);
        if (!f) return;
        std::string header; std::getline(f, header);

        auto parseRow = [](const std::vector<std::string>& cols, DiffCurve& out) -> bool {
            if (cols.size() < 7) return false;
            try {
                out.stormBias = std::stof(cols[1]);
                out.discMul = std::stof(cols[2]);
                out.forageMul = std::stof(cols[3]);
                out.waterMul = std::stof(cols[4]);
                out.animalMul = std::stof(cols[5]);
                out.weatherMul = std::stof(cols[6]);
            }
            catch (...) { return false; }
            return true;
            };

        std::string line;
        while (std::getline(f, line)) {
            if (line.empty() || line[0] == '#') continue;
            std::stringstream ss(line);
            std::vector<std::string> cols; std::string tok;
            while (std::getline(ss, tok, '\t')) cols.push_back(tok);
            std::string lvl = cols[0];
            for (auto& c : lvl) c = (char)std::toupper((unsigned char)c);

            DiffCurve out{};
            if (!parseRow(cols, out)) continue;

            if (lvl == "EASY")   g_easy = out;
            if (lvl == "NORMAL") g_normal = out;
            if (lvl == "HARD")   g_hard = out;
        }
    }

    static DiffCurve blendedCurve(float tension) {
        auto lerp = [](float a, float b, float t) { return a + (b - a) * t; };

        if (tension >= 0.66f) {
            float t = (tension - 0.66f) / 0.34f; if (t < 0) t = 0; if (t > 1) t = 1;
            DiffCurve o{
                lerp(g_normal.stormBias, g_easy.stormBias, t),
                lerp(g_normal.discMul,   g_easy.discMul,   t),
                lerp(g_normal.forageMul, g_easy.forageMul, t),
                lerp(g_normal.waterMul,  g_easy.waterMul,  t),
                lerp(g_normal.animalMul, g_easy.animalMul, t),
                lerp(g_normal.weatherMul,g_easy.weatherMul,t)
            }; return o;
        }
        else if (tension <= 0.33f) {
            float t = (0.33f - tension) / 0.33f; if (t < 0) t = 0; if (t > 1) t = 1;
            DiffCurve o{
                lerp(g_normal.stormBias, g_hard.stormBias, t),
                lerp(g_normal.discMul,   g_hard.discMul,   t),
                lerp(g_normal.forageMul, g_hard.forageMul, t),
                lerp(g_normal.waterMul,  g_hard.waterMul,  t),
                lerp(g_normal.animalMul, g_hard.animalMul, t),
                lerp(g_normal.weatherMul,g_hard.weatherMul,t)
            }; return o;
        }
        return g_normal;
    }

    // Chain tracking + raft debug flag
    static int  g_activeChain = -1;
    static int  g_chainDaysLeft = 0;
    static bool g_forceRaft = false;

    static int parseChainId(const std::string& desc) {
        size_t a = desc.find("{chain:");
        if (a == std::string::npos) return -1;
        size_t b = desc.find('}', a + 7);
        if (b == std::string::npos) return -1;
        std::string num = desc.substr(a + 7, b - (a + 7));
        try { return std::stoi(num); }
        catch (...) { return -1; }
    }

} 

//  Weather overlay visuals 
static void renderWeatherEffects(SDL_Renderer* renderer, int w, int h,
    Weather weather, float& lightningFlash,
    std::mt19937& rng)
{
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    if (weather == Weather::Clear) {
        SDL_SetRenderDrawColor(renderer, 255, 220, 120, 30);
        for (int r = 60; r > 0; r -= 6) {
            SDL_FRect ring{ (float)w - 80.0f - r, 20.0f - r, (float)r * 2.0f, (float)r * 2.0f };
            SDL_RenderFillRect(renderer, &ring);
        }
        return;
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 20, (weather == Weather::Rain ? 80 : 120));
    SDL_FRect tint{ 0,0,(float)w,(float)h }; SDL_RenderFillRect(renderer, &tint);

    SDL_SetRenderDrawColor(renderer, 100, 180, 255, 170);
    int drops = (weather == Weather::Rain ? 120 : 180);
    float speed = (weather == Weather::Rain ? 7.0f : 10.0f);
    float offset = frand(rng, 0.0f, 10.0f);

    for (int i = 0; i < drops; ++i) {
        float x = frand(rng, 0.0f, (float)w);
        float y = fmodf(x * 0.3f + offset * speed * 12.0f + (float)i * 11.0f, (float)h);
        SDL_RenderLine(renderer, x, y, x - 2.0f, y + 8.0f);
    }

    if (weather == Weather::Storm) {
        if (irand(rng, 0, 120) == 0) lightningFlash = 1.0f;
        if (lightningFlash > 0.01f) {
            Uint8 a = (Uint8)(220 * lightningFlash);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, a);
            SDL_FRect flash{ 0,0,(float)w,(float)h }; SDL_RenderFillRect(renderer, &flash);
            lightningFlash *= 0.85f;
        }
        else {
            lightningFlash = 0.0f;
        }
    }
}

//  Crafting menu 
static void craftMenu(Player& player, WorldState& world)
{
    std::cout << "\n-- Crafting --\n";
    std::cout << "1) Campfire (cost: 2 wood)  " << (world.hasCampfire ? "[built]\n" : "\n");
    std::cout << "2) Rain Collector (cost: 3 wood)  " << (world.hasCollector ? "[built]\n" : "\n");
    std::cout << "3) Cancel\n> ";
    std::string line; std::getline(std::cin, line); if (line.empty()) std::getline(std::cin, line);
    int choice = 0; try { choice = std::stoi(line); }
    catch (...) {}

    switch (choice) {
    case 1:
        if (world.hasCampfire) { std::cout << "You already have a campfire.\n"; break; }
        if (player.wood >= 2) { player.wood -= 2; world.hasCampfire = true; std::cout << "Campfire built. +10 energy daily.\n"; }
        else std::cout << "Not enough wood.\n";
        break;
    case 2:
        if (world.hasCollector) { std::cout << "You already have a rain collector.\n"; break; }
        if (player.wood >= 3) { player.wood -= 3; world.hasCollector = true; std::cout << "Rain collector built. +1 water on rainy days.\n"; }
        else std::cout << "Not enough wood.\n";
        break;
    default: std::cout << "No item crafted.\n"; break;
    }
}

//  ctor / dtor 
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

    tryLoadDifficultyTSV(dataDir_ + "/difficulty.tsv");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n"; std::exit(1);
    }
    window_ = SDL_CreateWindow("Survivor Island HUD", 800, 500, SDL_WINDOW_RESIZABLE);
    renderer_ = SDL_CreateRenderer(window_, nullptr);
    if (!window_ || !renderer_) {
        std::cerr << "SDL create failed: " << SDL_GetError() << "\n"; std::exit(1);
    }

    renderHUD();
}

Game::~Game() {
    if (renderer_) SDL_DestroyRenderer(renderer_);
    if (window_)   SDL_DestroyWindow(window_);
    SDL_Quit();
}

//  Console UI 
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
    n(1);  std::cout << "Forage (food)\n";
    n(2);  std::cout << "Collect water\n";
    n(3);  std::cout << "Gather wood\n";
    n(4);  std::cout << "Explore (random event)\n";
    n(5);  std::cout << "Rest\n";
    n(6);  std::cout << "Build shelter (cost: 5 wood)\n";
    n(7);  std::cout << "Eat (-1 food +10 energy)\n";
    n(8);  std::cout << "Drink (-1 water +10 energy)\n";
    n(9);  std::cout << "Save & Quit\n";
    n(10); std::cout << "Craft item\n";
    std::cout << "> ";
}

//  Actions 
void Game::doAction(int c) {
    switch (c) {
    case 1: { // FORAGE 
        player_.energy -= 10;
        int got = 1;

        float pPlus1 = 0.22f;
        float pPlus2 = 0.06f;
        if (world_.weather == Weather::Clear)  pPlus1 += 0.05f;
        if (world_.phase == DayPhase::Morning) pPlus1 += 0.03f;

        DiffCurve curve = blendedCurve(director_.tension);
        pPlus1 *= curve.forageMul; pPlus2 *= curve.forageMul;
        if (director_.tension > 0.70f) { pPlus1 *= 1.20f; pPlus2 *= 1.15f; }
        auto clamp01 = [](float x) { return x < 0.f ? 0.f : (x > 0.95f ? 0.95f : x); };
        pPlus1 = clamp01(pPlus1); pPlus2 = clamp01(pPlus2);

        std::uniform_real_distribution<float> U(0.0f, 1.0f);
        float r = U(rng_);
        if (r < pPlus2)      got += 2;
        else if (r < pPlus1) got += 1;

        player_.food += got;
        break;
    }
    case 2: { // COLLECT WATER
        player_.energy -= 10;
        int got = 1;

        float pPlus1 = 0.25f, pPlus2 = 0.08f;
        if (world_.weather == Weather::Rain) { pPlus1 += 0.20f; pPlus2 += 0.06f; }
        if (world_.hasCollector) { pPlus1 += 0.10f; }

        DiffCurve curve = blendedCurve(director_.tension);
        pPlus1 *= curve.waterMul; pPlus2 *= curve.waterMul;
        if (director_.tension > 0.70f) { pPlus1 *= 1.20f; pPlus2 *= 1.15f; }
        auto clamp01 = [](float x) { return x < 0.f ? 0.f : (x > 0.95f ? 0.95f : x); };
        pPlus1 = clamp01(pPlus1); pPlus2 = clamp01(pPlus2);

        std::uniform_real_distribution<float> U(0.0f, 1.0f);
        float r = U(rng_);
        if (r < pPlus2)      got += 2;
        else if (r < pPlus1) got += 1;

        player_.water += got;
        break;
    }

    case 3: player_.energy -= 12; player_.wood += 2; break;
    case 4: player_.energy -= 8;  applyEvent();       break;
    case 5: player_.energy += (player_.hasShelter ? 25 : 15); break;
    case 6:
        if (player_.wood >= 5) {
            player_.wood -= 5; player_.hasShelter = true;
            setColor(ConsoleColor::Green); std::cout << "You build a simple shelter.\n"; resetColor();
        }
        else {
            setColor(ConsoleColor::Red); std::cout << "Not enough wood.\n"; resetColor();
        }
        break;
    case 7:
        if (player_.food > 0) { player_.food--;  player_.energy += 10; player_.daysSinceEat = 0; }
        else { setColor(ConsoleColor::Red); std::cout << "You have no food.\n";  resetColor(); }
        break;
    case 8:
        if (player_.water > 0) { player_.water--; player_.energy += 10; player_.daysSinceDrink = 0; }
        else { setColor(ConsoleColor::Red); std::cout << "You have no water.\n"; resetColor(); }
        break;
    case 9:
        try { FileIO::saveGame(dataDir_ + "/save.txt", player_); std::cout << "Game saved. Goodbye!\n"; }
        catch (const std::exception& e) { std::cerr << "Save failed: " << e.what() << "\n"; }
        std::exit(0);
    case 10:
        craftMenu(player_, world_); break;
    default:
        std::cout << "Invalid choice.\n"; break;
    }
    player_.clamp();
}

//  Weather helpers 
void Game::advanceWeather() {
    std::uniform_real_distribution<float> U(0.0f, 1.0f);
    float r = U(rng_);
    auto step = [&](float p0, float p1, float /*p2*/) {
        if (r < p0)      return 0;
        if (r < p0 + p1) return 1;
        return 2;
        };
    int nxt = 0;
    switch (world_.weather) {
    case Weather::Clear: nxt = step(0.70f, 0.20f, 0.10f); break;
    case Weather::Rain:  nxt = step(0.30f, 0.50f, 0.20f); break;
    case Weather::Storm: nxt = step(0.50f, 0.40f, 0.10f); break;
    }
    world_.weather = static_cast<Weather>(nxt);

    DiffCurve curve = blendedCurve(director_.tension);
    float bias = director_.weatherBias() + curve.stormBias;
    float rb = U(rng_);
    if (bias < -0.01f) {
        if (world_.weather == Weather::Storm && rb < 0.55f) world_.weather = Weather::Rain;
        else if (world_.weather == Weather::Rain && rb < 0.40f) world_.weather = Weather::Clear;
    }
    else if (bias > 0.01f) {
        if (world_.weather == Weather::Clear && rb < 0.25f) world_.weather = Weather::Rain;
        else if (world_.weather == Weather::Rain && rb < 0.15f) world_.weather = Weather::Storm;
    }
}

float Game::weatherWeightMul(const std::string& type) const {
    float mul = 1.0f;
    if (world_.weather == Weather::Clear) {
        if (type == "DISCOVERY" || type == "FORAGE") mul *= 1.2f;
    }
    else if (world_.weather == Weather::Rain) {
        if (type == "WATER")   mul *= 2.0f;
        if (type == "FORAGE")  mul *= 1.1f;
        if (type == "WEATHER") mul *= 1.2f;
    }
    else { // Storm
        if (type == "WEATHER")   mul *= 2.0f;
        if (type == "ANIMAL")    mul *= 1.3f;
        if (type == "DISCOVERY") mul *= 0.8f;
        if (type == "FORAGE")    mul *= 0.9f;
    }

    DiffCurve curve = blendedCurve(director_.tension);
    if (type == "DISCOVERY") mul *= curve.discMul;
    else if (type == "FORAGE")    mul *= curve.forageMul;
    else if (type == "WATER")     mul *= curve.waterMul;
    else if (type == "ANIMAL")    mul *= curve.animalMul;
    else if (type == "WEATHER")   mul *= curve.weatherMul;

    mul *= director_.weightMultiplierForType(type);
    return mul;
}

//  Events  
void Game::applyEvent() {
    // Build per-event effective weights
    std::vector<int> eff; eff.reserve(events_.size() + 1);
    int totalW = 0;
    for (auto& e : events_) {
        int w = (int)std::floor(e.weight * weatherWeightMul(e.type) + 0.5f);
        if (w < 1) w = 1;

        if (g_activeChain >= 0 && g_chainDaysLeft > 0) {
            int cid = parseChainId(e.description);
            if (cid == g_activeChain) w = (int)std::floor(w * 1.45f);
            else                      w = (int)std::floor(w * 0.92f);
        }
        eff.push_back(w); totalW += w;
    }

    // Synthetic "Raft Escape"  if eligible or forced via debug
    bool raftEligible = (player_.wood >= 30);
    bool raftRoll = (irand(rng_, 0, 99) == 0); // 1% base chance
    bool offerRaft = raftEligible && (raftRoll || g_forceRaft);
    int raftWeightIdx = -1;
    if (offerRaft) {
        int w = 25; 
        eff.push_back(w);
        totalW += w;
        raftWeightIdx = (int)eff.size() - 1;
    }

    std::uniform_int_distribution<int> dist(1, totalW);
    int r = dist(rng_);

    const Event* picked = nullptr;
    bool pickedRaft = false;
    int cum = 0;
    for (size_t i = 0; i < eff.size(); ++i) {
        cum += eff[i];
        if (r <= cum) {
            if (raftWeightIdx >= 0 && (int)i == raftWeightIdx) pickedRaft = true;
            else picked = &events_[i];
            break;
        }
    }
    if (!picked && !pickedRaft) return;

	// Raft branch (escape event)
    if (pickedRaft) {
        g_forceRaft = false;
        setColor(ConsoleColor::Cyan);
        std::cout << "\n>> You discover sturdy driftwood and calm seas. "
            "With enough supplies you could lash a raft and try to escape...\n";
        resetColor();

        if (player_.wood < 30) {
            setColor(ConsoleColor::Red);
            std::cout << "You need 30 wood (you have " << player_.wood << ").\n";
            resetColor();
            return;
        }

        std::cout << "Build the raft and escape now? (-30 WOOD) [y/n]: ";
        std::string line; std::getline(std::cin, line); if (line.empty()) std::getline(std::cin, line);
        if (!line.empty() && (line[0] == 'y' || line[0] == 'Y')) {
            player_.wood -= 30; player_.clamp();
            WinAndQuit("You build a raft and ride a favorable current to shipping lanes.");
        }
        else {
            std::cout << "You decide to wait for a better moment.\n";
        }
        return;
    }

    // Normal picked event
    int score = picked->dEnergy + picked->dFood + picked->dWater + picked->dWood + picked->dHealth;
    ConsoleColor col = (score > 0) ? ConsoleColor::Green : (score < 0 ? ConsoleColor::Red : ConsoleColor::Gray);

    std::string cleanText = stripTags(picked->description);
    std::string deltas = buildDeltaString(*picked);
    setColor(col);
    std::cout << "\n>> " << cleanText << " " << deltas << "\n";
    resetColor();

    player_.energy += picked->dEnergy;
    player_.food += picked->dFood;
    player_.water += picked->dWater;
    player_.wood += picked->dWood;
    player_.health += picked->dHealth;
    player_.clamp();

    int newChain = parseChainId(picked->description);
    if (newChain >= 0) { g_activeChain = newChain; g_chainDaysLeft = 2; }

    director_.onEventApplied(*picked);
}

//  Tick to next day
void Game::nextDayTick() {
    player_.daysSinceEat++;
    player_.daysSinceDrink++;

    if (player_.daysSinceEat == 2) { setColor(ConsoleColor::Yellow); std::cout << "Your stomach growls. Consider eating soon.\n";  resetColor(); }
    if (player_.daysSinceDrink == 2) { setColor(ConsoleColor::Yellow); std::cout << "You feel parched. Consider drinking soon.\n"; resetColor(); }

    // Food
    if (player_.daysSinceEat >= 3) {
        if (player_.food > 0) {
            player_.food--; player_.daysSinceEat = 0;
            setColor(ConsoleColor::Green); std::cout << "You eat some food to keep going.\n"; resetColor();
        }
        else {
            int hpLoss = (player_.daysSinceEat >= 4) ? 12 : 6;
            int enLoss = (player_.daysSinceEat >= 4) ? 18 : 10;
            player_.health -= hpLoss; player_.energy -= enLoss;
            setColor(ConsoleColor::Red);
            std::cout << "You are starving (" << player_.daysSinceEat << " ticks without food). HP-" << hpLoss << " EN-" << enLoss << "\n";
            resetColor();
        }
    }

    // Water
    if (player_.daysSinceDrink >= 3) {
        if (player_.water > 0) {
            player_.water--; player_.daysSinceDrink = 0;
            setColor(ConsoleColor::Green); std::cout << "You drink water and feel better.\n"; resetColor();
        }
        else {
            int hpLoss = (player_.daysSinceDrink >= 4) ? 25 : 12;
            int enLoss = (player_.daysSinceDrink >= 4) ? 25 : 15;
            player_.health -= hpLoss; player_.energy -= enLoss;
            setColor(ConsoleColor::Red);
            std::cout << "You are dehydrated (" << player_.daysSinceDrink << " days without water). HP-"
                << hpLoss << " EN-" << enLoss << "\n";
            resetColor();
        }
    }

    if (player_.hasShelter) player_.energy += 5;

    // Crafting daily bonuses
    if (world_.hasCampfire)                                      player_.energy += 10;
    if (world_.hasCollector && world_.weather == Weather::Rain)  player_.water += 1;

    // Weather daily effects
    switch (world_.weather) {
    case Weather::Clear: if (player_.hasShelter) player_.energy += 2; break;
    case Weather::Rain:  if (!player_.hasShelter) player_.energy -= 2; else player_.energy += 1; break;
    case Weather::Storm: player_.energy -= 4; if (!player_.hasShelter) player_.health -= 5; break;
    }

    director_.onDayStart(player_, world_);

    if (g_chainDaysLeft > 0) { g_chainDaysLeft--; if (g_chainDaysLeft == 0) g_activeChain = -1; }

    advanceWeather();

    DayPhase nextPhase =
        (world_.phase == DayPhase::Morning) ? DayPhase::Day :
        (world_.phase == DayPhase::Day) ? DayPhase::Evening :
        (world_.phase == DayPhase::Evening) ? DayPhase::Night :
        DayPhase::Morning;

    bool finishedDay = (world_.phase == DayPhase::Night && nextPhase == DayPhase::Morning);
    world_.phase = nextPhase;

    if (finishedDay) {
        player_.day++;
        if (player_.day >= 100) {
            WinAndQuit("You survived 100 days. A rescue ship spots your signal.");
        }
    }

    player_.clamp();
    renderHUD();
}

//  Menu input 
int Game::readMenuChoice() {
    std::string line;
    for (;;) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_EVENT_QUIT) { std::cout << "\nWindow closed. Exiting.\n"; std::exit(0); }
            if (ev.type == SDL_EVENT_KEY_DOWN) {
                if (ev.key.scancode == SDL_SCANCODE_D) { debug_ = !debug_; renderHUD(); }
                if (debug_) {
                    if (ev.key.scancode == SDL_SCANCODE_F1) {
                        g_forceRaft = true;
                        setColor(ConsoleColor::Yellow);
                        std::cout << "[DEBUG] Raft event will be offered on your next Explore.\n";
                        resetColor();
                    }
                    if (ev.key.scancode == SDL_SCANCODE_F2) {
                        player_.day = 99;
                        world_.phase = DayPhase::Night;
                        setColor(ConsoleColor::Yellow);
                        std::cout << "[DEBUG] Fast-forwarded to end of Day 99 (Night). Advance time to trigger Day 100 win.\n";
                        resetColor();
                        renderHUD();
                    }
                }
            }
        }

        if (_kbhit()) {
            std::getline(std::cin, line);
            if (line.empty()) continue;
            try { return std::stoi(line); }
            catch (...) { std::cout << "Please enter a number.\n> "; std::cout.flush(); }
        }

        renderHUD();
        SDL_Delay(16);
    }
}

//  Main loop 
void Game::run() {
    std::cout << "Load previous game? (y/n): ";
    char yn; std::cin >> yn;
    if (yn == 'y' || yn == 'Y') {
        try { player_ = FileIO::loadSave(dataDir_ + "/save.txt"); std::cout << "Save loaded.\n"; }
        catch (...) { std::cout << "No valid save found; starting new.\n"; }
        renderHUD();
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // clear newline

    while (true) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_EVENT_QUIT) { std::cout << "\nWindow closed. Exiting.\n"; return; }
            if (ev.type == SDL_EVENT_KEY_DOWN) {
                if (ev.key.scancode == SDL_SCANCODE_D) { debug_ = !debug_; renderHUD(); }
                if (debug_) {
                    if (ev.key.scancode == SDL_SCANCODE_F1) {
                        g_forceRaft = true;
                        setColor(ConsoleColor::Yellow);
                        std::cout << "[DEBUG] Raft event will be offered on your next Explore.\n";
                        resetColor();
                    }
                    if (ev.key.scancode == SDL_SCANCODE_F2) {
                        player_.day = 99;
                        world_.phase = DayPhase::Night;
                        setColor(ConsoleColor::Yellow);
                        std::cout << "[DEBUG] Fast-forwarded to end of Day 99 (Night). Advance time to trigger Day 100 win.\n";
                        resetColor();
                        renderHUD();
                    }
                }
            }
        }

        printHeader();
        if (player_.isDead()) { std::cout << "\nYou collapse. The island claims another soul.\n"; break; }

        printMenu();
        std::cout.flush();
        int choice = readMenuChoice();

        doAction(choice);
        renderHUD();
        nextDayTick();
    }
}

//  Debug overlay 
void Game::renderDebugPanel(int w, int h) const {
    if (!debug_) return;

    SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 140);
    SDL_FRect panel{ (float)w - 240.0f, 20.0f, 220.0f, 226.0f }; 
    SDL_RenderFillRect(renderer_, &panel);

    // Weather probabilities 
    float pClear = 0.0f, pRain = 0.0f, pStorm = 0.0f;
    switch (world_.weather) {
    case Weather::Clear: pClear = 0.70f; pRain = 0.20f; pStorm = 0.10f; break;
    case Weather::Rain:  pClear = 0.30f; pRain = 0.50f; pStorm = 0.20f; break;
    case Weather::Storm: pClear = 0.50f; pRain = 0.40f; pStorm = 0.10f; break;
    }

    auto bar = [&](float y, Uint8 r, Uint8 g, Uint8 b, float p) {
        SDL_SetRenderDrawColor(renderer_, r, g, b, 255);
        SDL_FRect brect{ panel.x + 10.0f, y, p * (panel.w - 20.0f), 10.0f };
        SDL_RenderFillRect(renderer_, &brect);
        };

    ui::DrawText5x7(renderer_, panel.x + 10.0f, panel.y + 10.0f,
        "NEXT WEATHER", 2.0f, SDL_Color{ 255,255,255,255 });
    bar(panel.y + 26.0f, 255, 220, 80, pClear);
    bar(panel.y + 42.0f, 80, 180, 255, pRain);
    bar(panel.y + 58.0f, 255, 80, 80, pStorm);

    // Hunger / thirst meters
    auto meter = [&](float y, int v, SDL_Color c) {
        int vcap = v; if (vcap < 0) vcap = 0; if (vcap > 4) vcap = 4;
        float unitW = 20.0f, unitH = 8.0f, gap = 4.0f;
        for (int i = 0; i < 4; i++) {
            SDL_SetRenderDrawColor(renderer_,
                (i < vcap) ? c.r : 80,
                (i < vcap) ? c.g : 80,
                (i < vcap) ? c.b : 80,
                255);
            SDL_FRect u{ panel.x + 10.0f + i * (unitW + gap), y, unitW, unitH };
            SDL_RenderFillRect(renderer_, &u);
        }
        };

    ui::DrawText5x7(renderer_, panel.x + 10.0f, panel.y + 78.0f,
        "HUNGER", 2.0f, SDL_Color{ 255,255,255,255 });
    meter(panel.y + 94.0f, player_.daysSinceEat, SDL_Color{ 255,220,80,255 });
    ui::DrawText5x7(renderer_, panel.x + 10.0f, panel.y + 108.0f,
        "THIRST", 2.0f, SDL_Color{ 255,255,255,255 });
    meter(panel.y + 124.0f, player_.daysSinceDrink, SDL_Color{ 100,180,255,255 });

    // Phase + general info
    const char* phaseName =
        (world_.phase == DayPhase::Morning) ? "MORN" :
        (world_.phase == DayPhase::Day) ? "DAY" :
        (world_.phase == DayPhase::Evening) ? "EVE" : "NIGHT";

    ui::DrawText5x7(renderer_, panel.x + 10.0f, panel.y + 142.0f,
        std::string("DAY:") + std::to_string(player_.day) +
        " HP:" + std::to_string(player_.health) +
        " PH:" + phaseName,
        2.0f, SDL_Color{ 230,230,230,255 });

    std::string crafts =
        std::string("CF:") + (world_.hasCampfire ? "Y" : "N") +
        " RC:" + (world_.hasCollector ? "Y" : "N");
    ui::DrawText5x7(renderer_, panel.x + 148.0f, panel.y + 142.0f,
        crafts, 2.0f, SDL_Color{ 230,230,230,255 });

    DiffCurve curve = blendedCurve(director_.tension);
    ui::DrawText5x7(renderer_, panel.x + 10.0f, panel.y + 160.0f,
        "DIR TENSION", 2.0f, SDL_Color{ 255,255,255,255 });
    float t = director_.tension; if (t < 0.0f) t = 0.0f; if (t > 1.0f) t = 1.0f;
    SDL_SetRenderDrawColor(renderer_, 255, 200, 100, 255);
    SDL_FRect tr{ panel.x + 10.0f, panel.y + 176.0f, t * (panel.w - 20.0f), 8.0f };
    SDL_RenderFillRect(renderer_, &tr);

    std::string line = "CHAIN:";
    line += (g_activeChain >= 0 ? std::to_string(g_activeChain) : "-");
    line += "  CURVE(sb=" + std::to_string((int)std::round(curve.stormBias * 100)) + "%)";
    ui::DrawText5x7(renderer_, panel.x + 10.0f, panel.y + 190.0f,
        line, 2.0f, SDL_Color{ 220,220,220,255 });

    // Debug hotkeys legend
    ui::DrawText5x7(renderer_, panel.x + 10.0f, panel.y + 206.0f,
        "F1: Force Raft  F2: Jump to D99", 2.0f, SDL_Color{ 220,220,220,255 });
}

//  HUD rendering 
void Game::renderHUD() {
    static float prevFood = (float)0;
    static float prevWater = (float)0;
    static float prevWood = (float)0;
    static float animTimer = 0.0f; animTimer += 0.05f;

    bool pulseFood = (player_.food != (int)prevFood);
    bool pulseWater = (player_.water != (int)prevWater);
    bool pulseWood = (player_.wood != (int)prevWood);
    if (pulseFood)  prevFood = (float)player_.food;
    if (pulseWater) prevWater = (float)player_.water;
    if (pulseWood)  prevWood = (float)player_.wood;

    SDL_Color bg = bgForPhase(world_.phase);
    SDL_SetRenderDrawColor(renderer_, bg.r, bg.g, bg.b, 255);
    SDL_RenderClear(renderer_);

    int w, h; SDL_GetWindowSize(window_, &w, &h);

    renderWeatherEffects(renderer_, w, h, world_.weather, world_.lightningFlash, rng_);

    // Health bar
    float barX = 20.0f, barY = 20.0f, barWidth = (float)(w - 40); if (barWidth < 120.0f) barWidth = 120.0f;
    float barHeight = 30.0f, filled = barWidth * (player_.health / 100.0f);
    SDL_FRect bgbar{ barX, barY, barWidth, barHeight };
    SDL_SetRenderDrawColor(renderer_, 60, 60, 60, 255); SDL_RenderFillRect(renderer_, &bgbar);
    SDL_SetRenderDrawColor(renderer_, (Uint8)(255 - (int)(player_.health * 2.55f)), (Uint8)((int)(player_.health * 2.55f)), 0, 255);
    SDL_FRect fg{ barX, barY, filled, barHeight }; SDL_RenderFillRect(renderer_, &fg);
    ui::DrawText5x7(renderer_, barX, barY - 16.0f, std::string("HEALTH ") + std::to_string(player_.health) + "/100", 2.0f, SDL_Color{ 255,255,255,255 });

    // Energy bar (yellow) below health
    float eBarY = barY + barHeight + 18.0f;
    float eBarH = 20.0f;
    float ePerc = player_.energy / 100.0f; if (ePerc < 0.0f) ePerc = 0.0f; if (ePerc > 1.0f) ePerc = 1.0f;
    float eFill = barWidth * ePerc;
    SDL_FRect eBg{ barX, eBarY, barWidth, eBarH };
    SDL_SetRenderDrawColor(renderer_, 40, 40, 40, 255);
    SDL_RenderFillRect(renderer_, &eBg);
    SDL_SetRenderDrawColor(renderer_, 255, 220, 80, 255);
    SDL_FRect eFg{ barX, eBarY, eFill, eBarH };
    SDL_RenderFillRect(renderer_, &eFg);
    ui::DrawText5x7(renderer_, barX, eBarY - 14.0f,
        std::string("ENERGY ") + std::to_string(player_.energy) + "/100",
        2.0f, SDL_Color{ 255,255,255,255 });

    // Weather icon + labels
    SDL_FRect icon{ (float)w - 60.0f, 22.0f, 38.0f, 26.0f };
    switch (world_.weather) {
    case Weather::Clear: {
        SDL_SetRenderDrawColor(renderer_, 255, 220, 80, 255);
        for (int y = -10; y < 10; ++y) for (int x = -10; x < 10; ++x)
            if (x * x + y * y < 80) SDL_RenderPoint(renderer_, icon.x + 19 + x, icon.y + 13 + y);
        break;
    }
    case Weather::Rain: {
        SDL_SetRenderDrawColor(renderer_, 200, 200, 200, 255);
        SDL_FRect cloud{ icon.x + 4, icon.y + 4, 30, 12 }; SDL_RenderFillRect(renderer_, &cloud);
        SDL_SetRenderDrawColor(renderer_, 100, 180, 255, 255);
        for (int i = 0; i < 3; i++) SDL_RenderLine(renderer_, icon.x + 10 + i * 7, icon.y + 18, icon.x + 8 + i * 7, icon.y + 24);
        break;
    }
    case Weather::Storm: {
        SDL_SetRenderDrawColor(renderer_, 120, 120, 120, 255);
        SDL_FRect cloud{ icon.x + 4, icon.y + 4, 30, 12 }; SDL_RenderFillRect(renderer_, &cloud);
        SDL_SetRenderDrawColor(renderer_, 255, 220, 80, 255);
        SDL_RenderLine(renderer_, icon.x + 18, icon.y + 16, icon.x + 12, icon.y + 24);
        SDL_RenderLine(renderer_, icon.x + 12, icon.y + 24, icon.x + 20, icon.y + 24);
        SDL_RenderLine(renderer_, icon.x + 20, icon.y + 24, icon.x + 16, icon.y + 30);
        break;
    }
    }

    const char* wname = (world_.weather == Weather::Clear) ? "CLEAR" : (world_.weather == Weather::Rain) ? "RAIN" : "STORM";
    const char* phname =
        (world_.phase == DayPhase::Morning) ? "MORNING" :
        (world_.phase == DayPhase::Day) ? "DAY" :
        (world_.phase == DayPhase::Evening) ? "EVENING" : "NIGHT";
    ui::DrawText5x7(renderer_, (float)w - 130.0f, barY + 4.0f, wname, 2.0f, SDL_Color{ 230,230,230,255 });
    ui::DrawText5x7(renderer_, barX + 220.0f, barY - 16.0f, phname, 2.0f, SDL_Color{ 230,230,230,255 });

    // Resource boxes
    float boxSize = 80.0f, padding = 20.0f, startY = 120.0f, startX = 20.0f;
    struct Item { const char* label; int value; SDL_Color color; bool pulse; };
    Item items[] = {
        {"FOOD",  player_.food,  SDL_Color{200,200, 50,255},   pulseFood  },
        {"WATER", player_.water, SDL_Color{ 80,180,255,255},   pulseWater },
        {"WOOD",  player_.wood,  SDL_Color{139, 69, 19,255},   pulseWood  }
    };

    for (int i = 0; i < 3; ++i) {
        SDL_FRect box{ startX + i * (boxSize + padding), startY, boxSize, boxSize };
        SDL_SetRenderDrawColor(renderer_, 40, 40, 40, 255); SDL_RenderFillRect(renderer_, &box);

        float pulse = 1.0f + (items[i].pulse ? 0.2f * std::sin(animTimer * 8.0f) : 0.0f);
        Uint8 r = clampU8((int)(items[i].color.r * pulse));
        Uint8 g = clampU8((int)(items[i].color.g * pulse));
        Uint8 b = clampU8((int)(items[i].color.b * pulse));

        SDL_SetRenderDrawColor(renderer_, r, g, b, 255);
        SDL_FRect inner{ box.x + 8.0f, box.y + 8.0f, box.w - 16.0f, box.h - 16.0f }; SDL_RenderFillRect(renderer_, &inner);

        SDL_SetRenderDrawColor(renderer_, 255, 255, 255, 255); SDL_RenderRect(renderer_, &box);
        ui::DrawText5x7(renderer_, box.x + 4.0f, box.y - 14.0f, items[i].label, 2.0f, SDL_Color{ 240,240,240,255 });

        std::string v = std::to_string(items[i].value);
        float tw = (float)v.size() * 6.0f * 3.0f;
        ui::DrawText5x7(renderer_, box.x + (box.w - tw) / 2.0f, box.y + box.h + 16.0f, v, 3.0f, SDL_Color{ 255,255,255,255 });
    }

    renderDebugPanel(w, h);
    SDL_RenderPresent(renderer_);
}
