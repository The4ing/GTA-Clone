#pragma once

#include <SFML/Graphics.hpp>
#include "Inventory.h"
#include "Player.h"

class InventoryUI {
public:
    InventoryUI();
    void draw(sf::RenderWindow& window, const Inventory& inventory);
    void handleInput(Player& player, Inventory& inףventory, sf::RenderWindow& window);

private:
    std::vector<sf::FloatRect> itemRects;
    int selectedIndex ;
    sf::Font font;
   
};
