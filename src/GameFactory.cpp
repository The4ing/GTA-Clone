// GameFactory.cpp
#include "GameFactory.h"
#include "HealthPresent.h"
#include "WeaponPresent.h"
#include "SpeedBoost.h"
#include "AmmoPresent.h"
#include "CollisionUtils.h"
#include "ResourceManager.h"
#include <iostream>

std::unique_ptr<Player> GameFactory::createPlayer(const sf::Vector2f& pos) {
    auto player = std::make_unique<Player>();
    player->setPosition(pos);
    return player;
}

std::unique_ptr<CarManager> GameFactory::createCarManager(const std::vector<RoadSegment>& roads) {

    auto carManager = std::make_unique<CarManager>();
    carManager->setRoads(roads);
    carManager->buildRoadTree();
    return carManager;
}

std::unique_ptr<PoliceManager> GameFactory::createPoliceManager(std::vector<std::vector<sf::Vector2f>> blockedPolygons) {
    auto policeManager = std::make_unique<PoliceManager>();
    const int numInitialPolice = 10;
    int attempts = 0;

    for (int i = 0; i < numInitialPolice && attempts < 1000; ) {
        ++attempts;
        float x = static_cast<float>(rand() % MAP_WIDTH);
        float y = static_cast<float>(rand() % MAP_HEIGHT);
        sf::Vector2f pos(x, y);

        if (!isBlocked(pos, blockedPolygons)) {
            policeManager->spawnPolice(pos);
            ++i; // רק אם הצלחנו למקם
        }
    }

    return policeManager;
}


std::unique_ptr<PedestrianManager> GameFactory::createPedestrianManager(std::vector<std::vector<sf::Vector2f>> blockedPolygons) {
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

std::vector<std::unique_ptr<Present>> GameFactory::createPresents(int count, const std::vector<std::vector<sf::Vector2f>> blockedPolygons)
{
    std::vector<std::unique_ptr<Present>> result;
    int attempts = 0;
    const int maxAttempts = 1000;

    while (result.size() < count && attempts < maxAttempts) {
        ++attempts;

        int type = rand() % 4;
        float x = static_cast<float>(rand() % MAP_WIDTH);
        float y = static_cast<float>(rand() % MAP_HEIGHT);
        sf::Vector2f pos(x, y);

        if (isBlocked(pos, blockedPolygons))
            continue;

        switch (type) {
        case 0:
                //result.push_back(std::make_unique<HealthPresent>(
                //ResourceManager::getInstance().getTexture("Health"), pos));
            break;
        case 1:
               result.push_back(std::make_unique<WeaponPresent>(
               ResourceManager::getInstance().getTexture("pistol"), sf::Vector2f (50.f,50.f)));
               std::cout << "Present created at: (" << pos.x << ", " << pos.y << ")\n";
            break;
        case 2:
               // result.push_back(std::make_unique<SpeedBoost>(
                //ResourceManager::getInstance().getTexture("Speed"), pos));
            break;
        case 3:
                //result.push_back(std::make_unique<AmmoPresent>(
                //ResourceManager::getInstance().getTexture("Ammo"), pos));
            break;
        }
    }

    return result;
}


bool GameFactory::isBlocked(const sf::Vector2f& pos, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) {
    for (const auto& polygon : blockedPolygons) {
        if (CollisionUtils::pointInPolygon(pos, polygon)) {
            return true;
        }
    }
    return false;
}
