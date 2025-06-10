// GameFactory.h
#pragma once
#include <memory>
#include "Player.h"
#include "CarManager.h"
#include "PoliceManager.h"
#include "ChunkManager.h"

class GameFactory {
public:
    static std::unique_ptr<Player> createPlayer(const sf::Vector2f& pos);
    static std::unique_ptr<CarManager> createCarManager(const std::vector<RoadSegment>& roads);
    static std::unique_ptr<PoliceManager> createPoliceManager();
    static std::unique_ptr<ChunkManager> createChunkManager();
};
