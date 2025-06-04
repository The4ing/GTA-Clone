#pragma once
#include "Menu.h"
#include <memory>
#include <SFML/Graphics.hpp>
#include <iostream>
#include "player.h"
#include "ChunkManager.h"
#include "RoadSegment.h"
#include "Vehicle.h"
#include <cstdlib>  // rand()
#include "QuadTree.h"

using namespace std;

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
    std::vector<sf::FloatRect> blockedAreas;  
    std::vector<RoadSegment> roads;
    std::vector<Vehicle> vehicles;  

    QuadTree<RoadSegment> roadTree{ sf::FloatRect(0, 0, 4640, 4672) };  // ?? ?? ???? ??? ?? ???? ???
    void buildRoadTree();
    std::vector<RoadSegment> findNearbyRoads(const sf::FloatRect& area);
    void handleVehicleTurning(Vehicle& car);
    bool shouldTurnTo(const Vehicle& vehicle, const RoadSegment& candidateRoad);
private:
    void processEvents();
    void update(float dt);
    void render();
    void startGameFullscreen();
    void loadCollisionRectsFromJSON(const std::string& filename);
    void spawnSingleVehicleOnRoad();
    std::string getActualLaneDirection(const RoadSegment& road, int laneIndex);

    bool isNearEdge(const Vehicle& v, const RoadSegment& road);
    bool isRightTurn(const std::string& from, const std::string& to);
    bool isLeftTurn(const std::string& from, const std::string& to);
    bool isStraight(const std::string& from, const std::string& to);

    std::vector<std::vector<sf::Vector2f>> blockedPolygons;

    sf::RenderWindow            window;
    sf::View                    gameView;
    std::unique_ptr<Menu>       menu;
    std::unique_ptr<ChunkManager> chunkManager;
    std::unique_ptr<Player>     player;
    GameState                   currentState;
    sf::Clock                   clock;

};

