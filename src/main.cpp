#include "GameManager.h"
#include "ResourceInitializer.h"
#include "SoundManager.h" 
#include <iostream>
#include <SFML/System/Sleep.hpp> 

int main() {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    try {
        ResourceInitializer::loadAll();
       // SoundManager::getInstance().playSound("gunshot"); // Example: Play startup sound
        // sf::sleep(sf::seconds(1)); //  for testing
        GameManager game;
        game.run();
    }
    catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << '\n';
        return 1;
    }
    return 0;
}
