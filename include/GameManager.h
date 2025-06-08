#pragma once
#include "Menu.h"
#include <memory>
#include <SFML/Graphics.hpp>
#include <iostream>
#include "player.h"
#include "ChunkManager.h"
#include "CarManager.h"
#include "PoliceManager.h"

const unsigned int WINDOW_WIDTH = 1920;
const unsigned int WINDOW_HEIGHT = 1080;
const unsigned int MAP_WIDTH = 4640;
const unsigned int MAP_HEIGHT = 4672;

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
    void update(float dt);
    void render();
    void startGameFullscreen();
    void loadCollisionRectsFromJSON(const std::string& filename);

    std::vector<std::vector<sf::Vector2f>> blockedPolygons;
    std::vector<RoadSegment> roads;

    sf::RenderWindow            window;
    sf::View                    gameView;
    std::unique_ptr<Menu>       menu;
    std::unique_ptr<ChunkManager> chunkManager;
    std::unique_ptr<Player>     player;
    std::unique_ptr<CarManager> carManager;
    std::unique_ptr<PoliceManager> policeManager;
    GameState                   currentState;
    sf::Clock                   clock;
};
