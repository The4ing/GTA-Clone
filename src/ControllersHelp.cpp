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
        "W or Up Arrow - Move Up",
        "A or Left Arrow - Move Left",
        "S or Down Arrow - Move Down",
        "D or Right Arrow - Move Right"
        "Space - Attack",
        "I - Switch Weapon + INVENTORY",
        "E - open store",
        "M - Open Map",
        "ESC - Pause/Exit",
        "F - get into the car"
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

        window.clear(sf::Color(30, 30, 30)); // כהה לקריאה נוחה
        window.draw(background); // ✅ ציור הרקע

        for (auto& line : lines)
            window.draw(line);

        window.display();
    }
}

