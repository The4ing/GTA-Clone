#include "InventoryUI.h"
#include "iostream"


void InventoryUI::draw(sf::RenderWindow& window, const Inventory& inventory) {
    sf::Font font;
    if (!font.loadFromFile("resources/Miskan.ttf")) {
        std::cerr << "Cannot load font.\n";
        return;
    }

    itemRects.clear();

    sf::RectangleShape background(sf::Vector2f(window.getSize()));
    background.setFillColor(sf::Color(0, 0, 0, 180));
    window.draw(background);

    sf::Text title("Inventory", font, 32);
    title.setPosition(100.f, 40.f);
    title.setFillColor(sf::Color::Yellow);
    window.draw(title);

    int y = 100;
    for (const auto& item : inventory.getAllItems()) {
        sf::RectangleShape box(sf::Vector2f(250.f, 28.f));
        box.setFillColor(sf::Color(50, 50, 50));
        box.setOutlineColor(sf::Color::White);
        box.setOutlineThickness(2.f);
        box.setPosition(100.f, static_cast<float>(y));
        window.draw(box);

        sf::Text text(item.first + " x" + std::to_string(item.second), font, 20);
        text.setPosition(110.f, static_cast<float>(y + 5));
        text.setFillColor(sf::Color::White);
        window.draw(text);

        itemRects.emplace_back(box.getGlobalBounds());
        y += 35;
    }

    sf::Text hint("Click item to use. Press ESC to return.", font, 20);
    hint.setFillColor(sf::Color::Green);
    hint.setPosition(50.f, static_cast<float>(y));
    window.draw(hint);

  


}


void InventoryUI::handleInput(Player& player, Inventory& inventory, sf::RenderWindow& window) {
    if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);

        int index = 0;
        for (const auto& pair : inventory.getAllItems()) {
            if (index < itemRects.size() && itemRects[index].contains(static_cast<sf::Vector2f>(mousePos))) {
                std::string itemName = pair.first;
                if (inventory.useItem(itemName)) {
                    if (itemName == "Health")
                        player.heal(25);
                    else if (itemName == "Ammo")
                        std::cout << "Ammo used\n";
                    else if (itemName == "SpeedBoost")
                        std::cout << "Speed boost used\n";
                }
                break;
            }
            ++index;
        }
    }
    // תוכל להרחיב עם Num2, Num3 וכו'
}