#pragma once

#include <unordered_map>
#include <vector>
#include <utility>
#include <SFML/Graphics.hpp>
#include "Chunk.h"
#include "Constants.h"

struct IntPairHash {
    std::size_t operator()(const std::pair<int, int>& p) const noexcept {
        std::uint64_t key = (static_cast<std::uint64_t>(p.first) << 32)
            ^ static_cast<std::uint64_t>(static_cast<int>(p.second));
        return std::hash<std::uint64_t>()(key);
    }
};

class ChunkManager {
public:
    ChunkManager();
    ~ChunkManager();

    void updateChunks(const sf::Vector2f& playerPos, const sf::View& view);
    void draw(sf::RenderTarget& target, const sf::View& view);
    void updateObjects(float dt, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons);
    //for police 
    std::vector<sf::Vector2i> getActiveChunkCoords() const;
private:
    Chunk* createChunk(int cx, int cy);
    int maxChunksX;
    int maxChunksY;
    std::unordered_map<std::pair<int, int>, Chunk*, IntPairHash> chunks;
    static constexpr int LOAD_RADIUS = 1;

   

};
