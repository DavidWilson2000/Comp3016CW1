#include <SDL3/SDL.h>
#include <iostream>
#include "Game.hpp"

int main(int, char**) {
    // SDL3 setup
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Survivor Island (SDL3 + Console)", 800, 450, SDL_WINDOW_RESIZABLE);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    if (!window || !renderer) {
        std::cerr << "SDL window/renderer failed: " << SDL_GetError() << "\n";
        if (renderer) SDL_DestroyRenderer(renderer);
        if (window) SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Draw simple background
    SDL_SetRenderDrawColor(renderer, 25, 30, 60, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);

    // Run text-based game
    try {
        Game game("data");
        game.run();
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal: " << e.what() << "\n";
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
