#include "Menu.h"
#include "ResourceManager.h"

Menu::Menu(sf::RenderWindow& win) : window(win) {
    font = ResourceManager::getInstance().getFont("main");

    std::vector<std::string> labels = { "Start Game", "Settings", "Exit" };

    for (size_t i = 0; i < labels.size(); ++i) {
        sf::Text text;
        text.setFont(font);
        text.setString(labels[i]);
        text.setCharacterSize(36);
        text.setPosition(100.f, 100.f + i * 60.f);
        text.setFillColor(i == 0 ? sf::Color::Yellow : sf::Color::White);
        options.push_back(text);
    }

    
}


void Menu::update() {
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
        moveUp();
        sf::sleep(sf::milliseconds(150));
    }
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
        moveDown();
        sf::sleep(sf::milliseconds(150));
    }
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Enter)) {
        optionChosen = true;
        sf::sleep(sf::milliseconds(150));
    }


}

void Menu::draw() {
    for (auto& option : options)
        window.draw(option);
}

void Menu::moveUp() {
    if (selectedIndex > 0) {
        options[selectedIndex].setFillColor(sf::Color::White);
        selectedIndex--;
        options[selectedIndex].setFillColor(sf::Color::Yellow);
    }
}

void Menu::moveDown() {
    if (selectedIndex < static_cast<int>(options.size()) - 1) {
        options[selectedIndex].setFillColor(sf::Color::White);
        selectedIndex++;
        options[selectedIndex].setFillColor(sf::Color::Yellow);
    }
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


