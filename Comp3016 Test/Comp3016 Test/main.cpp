#include "Game.hpp"
#include <iostream>

int main(int, char**) {
    try {
        Game game("data");
        game.run();
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
