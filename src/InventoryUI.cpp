#pragma once

#include <SFML/Graphics.hpp>
#include "Inventory.h"
#include "Player.h"
#include <functional>

class InventoryUI {
public:
    InventoryUI();
    void draw(sf::RenderWindow& window, const Inventory& inventory);
    void handleInput(Player& player, Inventory& inventory, sf::RenderWindow& window);

private:
    Player playerPtr;
    std::vector<sf::FloatRect> itemRects;
    int selectedIndex;
    sf::Font font;
    
};
