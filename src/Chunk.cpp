#include "Chunk.h"
#include "ResourceManager.h"
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <algorithm> 

Chunk::Chunk(int cx, int cy)
    : xIndex(cx), yIndex(cy), loaded(false)
{
    
    static bool seeded = false;
    if (!seeded) {
        std::srand(static_cast<unsigned int>(std::time(nullptr)));
        seeded = true;
    }
}

Chunk::~Chunk() {
    unload();
}

void Chunk::load() {
    if (loaded) return;
    loaded = true;

    sf::Texture& mapTex = ResourceManager::getInstance().getTexture("map"); 
        chunkSprite.setTexture(mapTex);

    int left = xIndex * CHUNK_SIZE;
    int top = yIndex * CHUNK_SIZE;
    int texW = static_cast<int>(mapTex.getSize().x);
    int texH = static_cast<int>(mapTex.getSize().y);

    int width = std::min(CHUNK_SIZE, texW - left);
    int height = std::min(CHUNK_SIZE, texH - top);

    chunkSprite.setTextureRect({ left, top, width, height });
    chunkSprite.setPosition(static_cast<float>(left), static_cast<float>(top));

    

    StaticObject* building = new StaticObject();
    {
        float bx = left + CHUNK_SIZE / 2.0f - 32.f;
        float by = top + CHUNK_SIZE / 2.0f - 32.f;
        building->setPosition({ bx, by });
    }
    staticObjs.push_back(building);

   
    // נניח שאתה רוצה ליצור 3 הולכי רגל לכל צ'אנק
    for (int i = 0; i < 3; ++i) {
        Pedestrian* ped = new Pedestrian();

        // מקם את ההולך רגל בתוך תחום הצ'אנק
        float x = static_cast<float>(xIndex * CHUNK_SIZE + rand() % CHUNK_SIZE);
         float y = static_cast<float>(yIndex * CHUNK_SIZE + rand() % CHUNK_SIZE);
         ped->setPosition({ x, y });

         peds.push_back(ped);
     }
    

    
}

void Chunk::unload() {
    if (!loaded) return;
    loaded = false;

    
    for (auto* obj : staticObjs) {
        delete obj;
    }
    staticObjs.clear();

    for (auto* ped : peds) {
        delete ped;
    }
    peds.clear();

    
}

void Chunk::draw(sf::RenderTarget& target) {
    if (!loaded) return;

    target.draw(chunkSprite);

    for (auto* obj : staticObjs) {
        obj->draw(target);
    }

    for (auto* ped : peds) {
        ped->draw(target);
    }
}

void Chunk::update(float dt) {
    if (!loaded) return;
   
    for (auto* ped : peds) {
        ped->update(dt);
    }
}

sf::Vector2f Chunk::getCenterPosition() const {
    
    return {
        static_cast<float>(xIndex * CHUNK_SIZE + CHUNK_SIZE / 2),
        static_cast<float>(yIndex * CHUNK_SIZE + CHUNK_SIZE / 2)
    };
}

bool Chunk::isLoaded() const {
    return loaded;
}

int Chunk::getXIndex() const {
    return xIndex;
}

int Chunk::getYIndex() const {
    return yIndex;
}
