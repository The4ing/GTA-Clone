#include "GameManager.h"
#include "ResourceInitializer.h"
#include <iostream>

int main() {
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
