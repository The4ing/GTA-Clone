#include "ControllersHelp.h"
#include "ResourceManager.h"

ControllersHelp::ControllersHelp(sf::RenderWindow& window)
    : window(window)
{
    font = ResourceManager::getInstance().getFont("main");
    background.setTexture(ResourceManager::getInstance().getTexture("background_set"));
    sf::Vector2u textureSize = background.getTexture()->getSize();
    sf::Vector2u windowSize = window.getSize();
    background.setScale(
        static_cast<float>(windowSize.x) / textureSize.x,
        static_cast<float>(windowSize.y) / textureSize.y
    );
    background.setPosition(0.f, 0.f);

    setupText();
}

void ControllersHelp::setupText() {
    std::vector<std::string> texts = {
        "W - Move Up",
        "A - Move Left",
        "S - Move Down",
        "D - Move Right",
        "Space - Attack",
        "1-7 - Switch Weapon",
        "E - Interact",
        "M - Open Map",
        "ESC - Pause/Exit"
    };

    for (size_t i = 0; i < texts.size(); ++i) {
        sf::Text text;
        text.setFont(font);
        text.setString(texts[i]);
        text.setCharacterSize(28);
        text.setFillColor(sf::Color::White);
        text.setPosition(80, 80 + i * 40);
        lines.push_back(text);
    }
}

void ControllersHelp::run() {
    sf::Event event;
    while (window.isOpen()) {
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed ||
                (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape))
                return;
        }

        window.clear(); // ניקוי
        window.draw(background); // ✅ ציור הרקע

        for (auto& line : lines)
            window.draw(line);

        window.display();
    }
}

