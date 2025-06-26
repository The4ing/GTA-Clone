#pragma once
#include "Police.h"
#include <vector>
#include <memory>
#include <unordered_map>
#include <SFML/System/Vector2.hpp>
#include "Constants.h"
#include "QuadTree.h"

class GameManager;
class PoliceManager {
public:
    PoliceManager(GameManager& gameManager); 

    void spawnPolice(const sf::Vector2f& position);
    void update(float dt, const sf::Vector2f& playerPos,
        const std::vector<std::vector<sf::Vector2f>>& blockedPolygons);

    void draw(sf::RenderTarget& window);
    void damageClosest(const sf::Vector2f& pos, int amount);


    void trySpawnRandomPoliceNear(const std::vector<sf::Vector2i>& activeChunks, const sf::Vector2f& playerPos);
    //void addActiveChunk(const sf::Vector2i& chunkCoord);
    //new

private:
    void spawnPoliceNearChunk(const sf::Vector2i& chunkCoord); // NEW
    GameManager& m_gameManager; // Store reference to GameManager

    std::vector<std::unique_ptr<Police>> policeUnits;
    std::vector<sf::Vector2i> activeChunks;  //NEW
    float spawnCooldown = 0.f;
};
