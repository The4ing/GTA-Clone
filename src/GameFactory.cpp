// GameFactory.cpp
#include "GameFactory.h"
#include "HealthPresent.h"
#include "Pistol.h"
#include "SpeedBoost.h"
#include "CollisionUtils.h"
#include "ResourceManager.h"
#include "Rifle.h"
#include "Minigun.h"
#include "Bazooka.h"
#include <iostream>
#include "GameManager.h" 
#include "Knife.h"
#include "Grenade.h"
#include "Store.h"


std::unique_ptr<Player> GameFactory::createPlayer(GameManager& gameManager, const sf::Vector2f& pos) { 
    auto player = std::make_unique<Player>(gameManager); // Pass gameManager to constructor
    player->setPosition(pos);
    return player;
}

std::unique_ptr<CarManager> GameFactory::createCarManager(const std::vector<RoadSegment>& roads, PoliceManager& policeMgr) {
    auto carManager = std::make_unique<CarManager>(policeMgr); // Pass PoliceManager reference
    carManager->setRoads(roads);
    carManager->buildRoadTree();
    return carManager;
}

std::unique_ptr<PoliceManager> GameFactory::createPoliceManager(GameManager& gameManager, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) {
    auto policeManager = std::make_unique<PoliceManager>(gameManager); // Pass GameManager
    const int numInitialPolice = 10;
    int attempts = 0;

    for (int i = 0; i < numInitialPolice && attempts < 1000; ) {
        ++attempts;
        float x = static_cast<float>(rand() % MAP_WIDTH);
        float y = static_cast<float>(rand() % MAP_HEIGHT);
        sf::Vector2f pos(x, y);

        if (!isBlocked(pos, blockedPolygons)) {
            policeManager->spawnPolice(pos, PoliceWeaponType::BATON);
            ++i; // רק אם הצלחנו למקם
        }
    }

    return policeManager;
}





std::unique_ptr<PedestrianManager> GameFactory::createPedestrianManager(const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) {
    auto pedestrianManager = std::make_unique<PedestrianManager>();
    const int numInitialPedestrians = 100;
    int attempts = 0;

    for (int i = 0; i < numInitialPedestrians && attempts < 3000; ) {
        ++attempts;
        float x = static_cast<float>(rand() % MAP_WIDTH);
        float y = static_cast<float>(rand() % MAP_HEIGHT);
        sf::Vector2f pos(x, y);

        if (!isBlocked(pos, blockedPolygons)) {
            pedestrianManager->spawnPedestrian(pos);
            ++i;
        }
    }

    return pedestrianManager;
}




//std::unique_ptr<ChunkManager> GameFactory::createChunkManager() {
//    return std::make_unique<ChunkManager>();
//}

std::vector<std::unique_ptr<Present>> GameFactory::createPresents(int count, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons)
{
    std::vector<std::unique_ptr<Present>> result;
    int attempts = 0;
    const int maxAttempts = 1000;

    while (result.size() < count && attempts < maxAttempts) {
        ++attempts;

        int type = rand() % 7;
        float x = static_cast<float>(rand() % MAP_WIDTH);
        float y = static_cast<float>(rand() % MAP_HEIGHT);
        sf::Vector2f pos(x, y);

        if (isBlocked(pos, blockedPolygons))
            continue;

      //  switch (type) {
    switch (type) {
        case 0:
                result.push_back(std::make_unique<HealthPresent>(
                    ResourceManager::getInstance().getTexture("Health"), pos));
            break;
        case 1:
            result.push_back(std::make_unique<Pistol>(
                ResourceManager::getInstance().getTexture("Pistol"), pos));
            break;
        case 2:
               result.push_back(std::make_unique<SpeedBoost>(
                   ResourceManager::getInstance().getTexture("Speed"), pos));
            break;
        
        case 3:
            result.push_back(std::make_unique<Rifle>(
                ResourceManager::getInstance().getTexture("Rifle"), pos));
            break;
        case 4:
            result.push_back(std::make_unique<Minigun>(
                ResourceManager::getInstance().getTexture("Minigun"), pos));
            break;
        case 5:
            result.push_back(std::make_unique<Bazooka>(
                ResourceManager::getInstance().getTexture("Bazooka"), pos));
                
            break;
        case 6:
            result.push_back(std::make_unique<Knife>(
                ResourceManager::getInstance().getTexture("Knife"), pos));

            break;
        case 7:
            result.push_back(std::make_unique<Grenade>(
                ResourceManager::getInstance().getTexture("Grenade"), pos));

            break;

        }
    }

    return result;
}

std::vector<std::unique_ptr<Store>> GameFactory::createStores(const std::vector<std::vector<sf::Vector2f>>& blockedPolygons)
{
    std::vector<std::unique_ptr<Store>> tempStores;

    const int maxAttempts = 1000;
    int attempts = 0;

    while (attempts < maxAttempts) {
        ++attempts;

        float x = static_cast<float>(rand() % MAP_WIDTH);
        float y = static_cast<float>(rand() % MAP_HEIGHT);
        sf::Vector2f storePos(x, y);

        if (!isBlocked(storePos, blockedPolygons)) {
            tempStores.push_back(std::make_unique<Store>(sf::Vector2f(50.f, 50.f)));
            break; // found a valid location
        }
    }

    if (tempStores.empty()) {
        std::cout << "Could not place a store after " << maxAttempts << " attempts.\n";
    }

    return tempStores;
}






bool GameFactory::isBlocked(const sf::Vector2f& pos, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) {
    for (const auto& polygon : blockedPolygons) {
        if (CollisionUtils::pointInPolygon(pos, polygon)) {
            return true;
        }
    }
    return false;
}
