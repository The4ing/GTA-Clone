#pragma once

#include <vector>
#include <SFML/Graphics.hpp>
#include "StaticObject.h"
#include "Pedestrian.h"



class Chunk {
public:
    Chunk(int cx, int cy);
    ~Chunk();

    void load();
    void unload();
    void draw(sf::RenderTarget& target);
    void update(float dt, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons);
    sf::Vector2f getCenterPosition() const;
    bool isLoaded() const;
    int getXIndex() const;
    int getYIndex() const;

private:
    int xIndex;
    int yIndex;
    bool loaded;
    sf::Sprite chunkSprite;
    std::vector<StaticObject*> staticObjs;
    std::vector<Pedestrian*> peds;
   

};
