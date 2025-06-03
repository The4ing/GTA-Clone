#include "GameManager.h"
#include "ResourceInitializer.h"
#include <iostream>

int main() {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    try {
        ResourceInitializer::loadAll();
        GameManager game;
        game.run();
    }
    catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << '\n';
        return 1;
    }
    return 0;
}
