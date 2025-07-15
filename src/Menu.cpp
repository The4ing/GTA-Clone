#include "Menu.h"
#include "ResourceManager.h"
#include "Settings.h"
#include "SoundManager.h"
#include <iostream>

Menu::Menu(sf::RenderWindow& win) : window(win) {
    font = ResourceManager::getInstance().getFont("main");

    background.setTexture(ResourceManager::getInstance().getTexture("background_menu"));

    sf::Vector2u textureSize = background.getTexture()->getSize();
    sf::Vector2u windowSize = window.getSize();

    background.setScale(
        static_cast<float>(windowSize.x) / textureSize.x,
        static_cast<float>(windowSize.y) / textureSize.y
    );

    background.setPosition(0.f, 0.f);

    std::vector<std::string> labels = { "Start Game", "Settings", "Exit" };

    for (size_t i = 0; i < labels.size(); ++i) {
        sf::Text text;
        text.setFont(font);
        text.setString(labels[i]);
        text.setCharacterSize(36);
        text.setPosition(100.f, 100.f + i * 60.f);
        text.setFillColor(sf::Color::White);
        options.push_back(text);
    }

    // ✅ נגן מוזיקת תפריט ברקע בלולאה
    menuLoop.setBuffer(ResourceManager::getInstance().getSoundBuffer("MenuSound"));
    menuLoop.setLoop(true);
    menuLoop.setVolume(SoundManager::getInstance().getVolume());
    menuLoop.play();
    SoundManager::getInstance().registerExternalSound(&menuLoop);
   
}

void Menu::update(sf::Event& event) {
    auto mousePos = sf::Mouse::getPosition(window);
    auto worldPos = window.mapPixelToCoords(mousePos);

    int hoveredIndex = -1;


    

    for (size_t i = 0; i < options.size(); ++i) {
        if (options[i].getGlobalBounds().contains(worldPos)) {
            hoveredIndex = static_cast<int>(i);
            options[i].setFillColor(sf::Color::Yellow);
            options[i].setCharacterSize(42);  // הגדל טקסט

            if (event.type == sf::Event::MouseButtonPressed &&
                event.mouseButton.button == sf::Mouse::Left) {

                if (options[i].getString() == "Settings") {
                    Settings settings(window);
                    while (!settings.shouldExit()) {
                        sf::Event settingsEvent;
                        while (window.pollEvent(settingsEvent)) {
                            if (settingsEvent.type == sf::Event::Closed)
                                window.close();

                            settings.handleEvent(settingsEvent);
                        }

                        settings.update();
                        window.clear();
                        settings.draw();
                        window.display();
                    }
                }
                else if (options[i].getString() == "Start Game") {
                    selectedIndex = static_cast<int>(i);
                    optionChosen = true;
                }
                else if (options[i].getString() == "Exit") {
                    selectedIndex = static_cast<int>(i);
                    optionChosen = true;
                }
            }
        }
        else {
            options[i].setFillColor(sf::Color::White);
            options[i].setCharacterSize(36);
        }
    }
}

void Menu::draw() {
    window.draw(background);
    for (auto& option : options)
        window.draw(option);
}

int Menu::getSelectedIndex() const {
    return selectedIndex;
}

bool Menu::isOptionChosen() const {
    return optionChosen;
}

std::string Menu::getSelectedOption() const {
    return options[selectedIndex].getString();
}

void Menu::reset() {
    optionChosen = false;
    selectedIndex = 0;
    std::cout << "Menu reset." << std::endl;
}
