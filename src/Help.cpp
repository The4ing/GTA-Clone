#include "Help.h"
#include "ControllersHelp.h"
#include "GameHelp.h"
#include "ResourceManager.h"

Help::Help(sf::RenderWindow& window)
    : window(window), selectedOption(-1), shouldClose(false)
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

    setupMenu();
}


void Help::setupMenu() {
    std::vector<std::string> texts = {
        "Controllers Help",
        "Game Help",
        "Back"
    };

    for (size_t i = 0; i < texts.size(); ++i) {
        sf::Text text;
        text.setFont(font);
        text.setString(texts[i]);
        text.setCharacterSize(36);
        text.setFillColor(sf::Color::White);
        text.setPosition(100, 100 + i * 70);
        menuOptions.push_back(text);
    }
}

void Help::run() {
    sf::Event event;
    while (window.isOpen() && !shouldClose) {
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed ||
                (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)) {
                shouldClose = true;
            }

            if (event.type == sf::Event::MouseButtonPressed)
                handleInput();
        }

        updateVisuals();
        window.clear();
        window.draw(background);  // רקע לפני התפריט
        for (auto& option : menuOptions)
            window.draw(option);
        window.display();

    }
}

void Help::handleInput() {
    auto mousePos = sf::Mouse::getPosition(window);
    for (size_t i = 0; i < menuOptions.size(); ++i) {
        if (menuOptions[i].getGlobalBounds().contains((float)mousePos.x, (float)mousePos.y)) {
            switch (i) {
            case 0: {
                ControllersHelp ch(window);
                ch.run();
                break;
            }
            case 1: {
                GameHelp gh(window);
                gh.run();
                break;
            }
            case 2:
                shouldClose = true;
                break;

            }
        }
    }
}

void Help::updateVisuals() {
    auto mousePos = sf::Mouse::getPosition(window);
    for (auto& option : menuOptions) {
        if (option.getGlobalBounds().contains((float)mousePos.x, (float)mousePos.y)) {
            option.setFillColor(sf::Color::Yellow);
        }
        else {
            option.setFillColor(sf::Color::White);
        }
    }
}
