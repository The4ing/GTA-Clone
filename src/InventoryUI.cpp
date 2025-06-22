#include "InventoryUI.h"
#include "InventoryUI.h"
#include <SFML/Graphics.hpp>
#include <iostream>
#include "ResourceManager.h"


InventoryUI::InventoryUI():selectedIndex(0)
{
    font = ResourceManager::getInstance().getFont("main");
   
}

void InventoryUI::draw(sf::RenderWindow& window, const Inventory& inventory) {
    itemRects.clear();

    // רקע שקוף כהה
    sf::RectangleShape blurOverlay(sf::Vector2f(window.getSize()));
    blurOverlay.setFillColor(sf::Color(0, 0, 0, 180));
    window.draw(blurOverlay);

    sf::Vector2f center(window.getSize().x / 2.f, window.getSize().y / 2.f);
    const float radius = 150.f;

    auto itemsVec = std::vector<std::pair<std::string, InventoryItem>>(
        inventory.getAllItems().begin(),
        inventory.getAllItems().end()
    );

    const int itemCount = static_cast<int>(itemsVec.size());
    const float angleStep = 2 * 3.14159f / std::max(itemCount, 1);

    for (int i = 0; i < itemCount; ++i) {
        const auto& item = itemsVec[i];
        const sf::Texture* texture = item.second.texture;
        if (!texture) continue;

        float angle = angleStep * i - 3.14159f / 2.f; // התחלה מלמעלה
        sf::Vector2f pos = center + sf::Vector2f(std::cos(angle), std::sin(angle)) * radius;

        bool selected = (i == selectedIndex);
        float scale = selected ? 0.22f : 0.15f;
        sf::Vector2f texSize(texture->getSize());
        sf::Vector2f scaledSize = texSize * scale;

        sf::Sprite icon(*texture);
        icon.setScale(scale, scale);
        icon.setOrigin(texSize.x / 2.f, texSize.y / 2.f);
        icon.setPosition(pos);
        icon.setColor(selected ? sf::Color::White : sf::Color(180, 180, 180));
        window.draw(icon);

        // שמירה לקליקים בעתיד
        itemRects.emplace_back(icon.getGlobalBounds());

        // הצגת בועה עם מספר
        if (item.second.count > 0) {
            // בועה קטנה — ימינה למעלה יחסית לאייקון
            sf::CircleShape bubble(13.f);
            bubble.setFillColor(sf::Color::Red);

            // Shift bubble to top-right corner of the icon
            sf::Vector2f bubblePos = pos;
            bubblePos.x += scaledSize.x / 2.f - 10.f;
            bubblePos.y -= scaledSize.y / 2.f - 10.f;

            bubble.setPosition(bubblePos);
            window.draw(bubble);

            sf::Text countText(std::to_string(item.second.count), font, 14);
            countText.setFillColor(sf::Color::White);
            countText.setPosition(bubblePos.x + 5.f, bubblePos.y + 2.f);
            window.draw(countText);
        }
    }

    // טקסט עזרה
    sf::Text hint("Use Left/Right to select, ENTER to use, ESC to return", font, 20);
    hint.setFillColor(sf::Color::Black);
    hint.setPosition(center.x - 180.f, center.y + radius + 50.f);
    window.draw(hint);
}






//----------
void InventoryUI::handleInput(Player& player, Inventory& inventory, sf::RenderWindow& window) {
    const int cols = 4;
    const int totalItems = static_cast<int>(inventory.getAllItems().size());
    static bool leftPressed = false, rightPressed = false, upPressed = false, downPressed = false, enterPressed = false;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
        if (!leftPressed) {
            selectedIndex = (selectedIndex - 1 + totalItems) % totalItems;
            leftPressed = true;
        }
    }
    else leftPressed = false;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
        if (!rightPressed) {
            selectedIndex = (selectedIndex + 1) % totalItems;
            rightPressed = true;
        }
    }
    else rightPressed = false;


    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
        if (!upPressed) {
            if (selectedIndex - cols >= 0)
                selectedIndex -= cols;
            upPressed = true;
        }
    }
    else upPressed = false;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
        if (!downPressed) {
            if (selectedIndex + cols < totalItems)
                selectedIndex += cols;
            downPressed = true;
        }
    }
    else downPressed = false;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Enter)) {
        if (!enterPressed) {
            int index = 0;
            for (const auto& pair : inventory.getAllItems()) {
                if (index == selectedIndex) {
                    std::string itemName = pair.first;
                    if (inventory.useItem(itemName)) {
                        if (itemName == "Health")
                            player.heal(25);
                        else if (itemName == "Radar")
                            std::cout << "Radar used\n";
                        else if (itemName == "Speed")
                            player.increaseSpeed();
                    }
                    break;
                }
                ++index;
            }
            enterPressed = true;
        }
    }
    else enterPressed = false;
}
