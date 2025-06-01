#pragma once
#include "Menu.h"
#include <memory>
#include <SFML/Graphics.hpp>
#include <iostream>
#include "player.h"
#include "ChunkManager.h"
using namespace std;

const unsigned int WINDOW_WIDTH = 1920;
const unsigned int WINDOW_HEIGHT = 1080;
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
    void startGameFullscreen();

private:
    sf::RenderWindow window;
    sf::Clock clock;

    std::unique_ptr<ChunkManager> chunkManager;
    std::unique_ptr<Player>       player;
    sf::View                      gameView;
    //menu
    GameState currentState = GameState::Menu;
    std::unique_ptr<Menu> menu;
};


