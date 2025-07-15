#include "GameHelp.h"
#include "ResourceManager.h"

GameHelp::GameHelp(sf::RenderWindow& window)
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

void GameHelp::setupText() {
    std::vector<std::string> texts = {
        "Welcome to the Game!",
        "Your goal is to complete missions and survive.",
        "",
        "Kill enemies to get weapons and money.",
        "Avoid the police when your wanted level increases.",
        "Use stores to buy ammo and gear.",
        "Use vehicles to move faster.",
        "Complete story missions to progress.",
        "",
        "Press ESC at any time to pause or exit."
    };

    for (size_t i = 0; i < texts.size(); ++i) {
        sf::Text text;
        text.setFont(font);
        text.setString(texts[i]);
        text.setCharacterSize(26);
        text.setFillColor(sf::Color::White);
        text.setPosition(80, 60 + i * 35);
        lines.push_back(text);
    }
}

void GameHelp::run() {
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
