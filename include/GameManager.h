#pragma once

#include "Menu.h"
#include <memory>
#include <SFML/Graphics.hpp>

enum class GameState {
    Menu,
    Playing,
    Exiting
};

class GameManager {
public:
    GameManager();
    void run();

private:
    void processEvents();
    void update(float deltaTime);
    void render();

private:
    sf::RenderWindow window;
    sf::Clock clock;
    const unsigned int WINDOW_WIDTH = 800;
    const unsigned int WINDOW_HEIGHT = 600;

    //menu
    GameState currentState = GameState::Menu;
    std::unique_ptr<Menu> menu;
};


