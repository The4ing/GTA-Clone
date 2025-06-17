// GameFactory.h
#pragma once
#include <memory>
#include "Player.h"
#include "CarManager.h"
#include "PoliceManager.h"
#include "ChunkManager.h"
#include "PedestrianManager.h"
#include "Present.h"

class GameFactory {
public:
    static std::unique_ptr<Player> createPlayer(const sf::Vector2f& pos);
    static std::unique_ptr<CarManager> createCarManager(const std::vector<RoadSegment>& roads);
    static std::unique_ptr<PoliceManager> createPoliceManager(std::vector<std::vector<sf::Vector2f>> blockedPolygons);
    static std::unique_ptr<PedestrianManager> createPedestrianManager(std::vector<std::vector<sf::Vector2f>> blockedPolygons);
    static std::vector<std::unique_ptr<Present>> createPresents(int count, const std::vector<std::vector<sf::Vector2f>> blockedPolygons);
    static bool isBlocked(const sf::Vector2f& pos, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons);

//    static std::unique_ptr<ChunkManager> createChunkManager();
};
