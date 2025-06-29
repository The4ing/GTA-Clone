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
#include <vector> // Added for std::vector
#include "BulletPool.h"
#include "PathfindingGrid.h" // Added for PathfindingGrid
#include "Store.h"


enum class GameState {
    Menu,
    Playing,
    Inventory,
    Store,
    Exiting
};


class GameManager {
public:
    GameManager();
    // void addBullet(std::unique_ptr<Bullet> bullet); // Old methodAdd commentMore actions
    void addBullet(const sf::Vector2f& startPos, const sf::Vector2f& direction); // New method for pool
    void run();
    PathfindingGrid* getPathfindingGrid() const;

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
    // std::vector<std::unique_ptr<Bullet>> bullets; // Replaced with BulletPoolAdd commentMore actions
    BulletPool                  bulletPool; // Added BulletPool instance
    sf::RenderWindow            window;
    sf::View                    gameView;
    std::unique_ptr<Menu>       menu;
    sf::Sprite mapSprite;
    sf::Texture* mapTexture = nullptr;
    //    std::unique_ptr<ChunkManager> chunkManager;
    std::vector<std::unique_ptr<Store>> store;
    std::unique_ptr<Player>     player;
    std::unique_ptr<CarManager> carManager;
    std::unique_ptr<PoliceManager> policeManager;
    std::unique_ptr<PedestrianManager> pedestrianManager;
    std::unique_ptr<PathfindingGrid> pathfindingGrid; // Added PathfindingGrid member
    std::vector<std::unique_ptr<Present>> presents;
    GameState                   currentState;
    sf::Clock                   clock;
    sf::Time                    m_gameTime;
    // HUD Members
    std::unique_ptr<HUD>        m_hud;
    sf::View                    m_hudView;
    static constexpr float GAME_TIME_SCALE = 60.0f;

    InventoryUI inventoryUI;
    sf::RenderTexture frozenBackgroundTexture;
    sf::Sprite frozenBackgroundSprite;

    int m_playingFrameCount = 0; // Counter for initial playing frames diagnostics
};
