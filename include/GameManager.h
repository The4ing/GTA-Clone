#pragma once
#include "Menu.h"
#include <memory>
#include <SFML/Graphics.hpp>
#include <iostream>
#include "player.h"
#include "HUD.h" 
//#include "ChunkManager.h"
#include "CarManager.h"
#include "PoliceManager.h"
#include "PedestrianManager.h"
#include "Present.h"
#include "InventoryUI.h"



enum class GameState {
    Menu,
    Playing,
    Inventory,
    Exiting
};


class GameManager {
public:
    GameManager();
    void run();

private:
    bool isFullscreen = false;
    void processEvents();
    void update(float dt);
    void render();
    void startGameFullscreen();
    void setFullscreen(bool fullscreen);
    void loadCollisionRectsFromJSON(const std::string& filename);
    void buildBlockedPolyTree();
    //FOR THE INVENTORY SHOWAdd commentMore actions
    void renderFrozenGame(sf::RenderTarget& target);

    std::vector<std::vector<sf::Vector2f>> blockedPolygons;
    QuadTree<std::vector<sf::Vector2f>> blockedPolyTree{ sf::FloatRect(0, 0, 4640, 4672) };
    std::vector<RoadSegment> roads;

    sf::RenderWindow            window;
    sf::View                    gameView;
    std::unique_ptr<Menu>       menu;
    sf::Sprite mapSprite;
    sf::Texture* mapTexture = nullptr;
//    std::unique_ptr<ChunkManager> chunkManager;
    std::unique_ptr<Player>     player;
    std::unique_ptr<CarManager> carManager;
    std::unique_ptr<PoliceManager> policeManager;
    std::unique_ptr<PedestrianManager> pedestrianManager;
    std::vector<std::unique_ptr<Present>> presents;
    GameState                   currentState;
    sf::Clock                   clock;
    sf::Time                    m_gameTime;
    // HUD Members
    std::unique_ptr<HUD>        m_hud;
    sf::View                    m_hudView;
    static constexpr float GAME_TIME_SCALE = 60.0f; // Each real second advances game time by 1 minute.

    InventoryUI inventoryUI;
    sf::RenderTexture frozenBackgroundTexture;
    sf::Sprite frozenBackgroundSprite;
};
