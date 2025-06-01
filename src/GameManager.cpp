#include "GameManager.h"

#include <iostream>

GameManager::GameManager()
    : window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Top-Down GTA Clone") {
    window.setFramerateLimit(60);

    // אתחול תפריט
    menu = std::make_unique<Menu>(window);
    currentState = GameState::Menu;
}

void GameManager::run() {
    while (window.isOpen()) {
        float deltaTime = clock.restart().asSeconds();
        processEvents();

        if (currentState == GameState::Menu) {
            menu->update();
            if (menu->isOptionChosen()) {
                std::string selected = menu->getSelectedOption();

                if (selected == "Start Game")
                    currentState = GameState::Playing;
                else if (selected == "Exit")
                    window.close();
                // Settings לא פעיל עדיין
            }

            window.clear();
            menu->draw();
            window.display();
        }

        else if (currentState == GameState::Playing) {
            update(deltaTime);
            render();
        }
    }
}


void GameManager::processEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed)
            window.close();
    }
}

void GameManager::update(float /*deltaTime*/) {
    
}

void GameManager::render() {
    window.clear();
    
    window.display();
}

