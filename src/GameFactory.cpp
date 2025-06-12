// GameFactory.cpp
#include "GameFactory.h"


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

std::unique_ptr<PoliceManager> GameFactory::createPoliceManager() {
    auto policeManager = std::make_unique<PoliceManager>();

   
    const int numInitialPolice = 10;

    for (int i = 0; i < numInitialPolice; ++i) {
        float x = static_cast<float>(rand() % MAP_WIDTH);
        float y = static_cast<float>(rand() % MAP_HEIGHT);

        policeManager->spawnPolice({ x, y });
    }

    return policeManager;
}

std::unique_ptr<PedestrianManager> GameFactory::createPedestrianManager() {
    auto pedestrianManager = std::make_unique<PedestrianManager>();

   
    const int numInitialPolice = 100;

    for (int i = 0; i < numInitialPolice; ++i) {
       
        float x = static_cast<float>(rand() % MAP_WIDTH);
        float y = static_cast<float>(rand() % MAP_HEIGHT);

        pedestrianManager->spawnPedestrian({ x, y });
    }

    return pedestrianManager;


}


//std::unique_ptr<ChunkManager> GameFactory::createChunkManager() {
//    return std::make_unique<ChunkManager>();
//}
