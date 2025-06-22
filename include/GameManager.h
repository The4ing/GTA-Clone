#pragma once
#include "Menu.h"
#include <memory>
#include <SFML/Graphics.hpp>
#include <iostream>
#include "player.h"
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

    //FOR THE INVENTORY SHOW
    void renderFrozenGame(sf::RenderTarget& target);
    ///--------------

    void update(float dt);
    void render();
    void startGameFullscreen();
    void setFullscreen(bool fullscreen);
    void loadCollisionRectsFromJSON(const std::string& filename);
    void buildBlockedPolyTree();
   

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


    ///-------------
    InventoryUI inventoryUI;
    sf::RenderTexture frozenBackgroundTexture;
    sf::Sprite frozenBackgroundSprite;

};
