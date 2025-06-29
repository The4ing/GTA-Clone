//#include "ChunkManager.h"
//
//#include <iostream>
//
//ChunkManager::ChunkManager() {}
//
//
//ChunkManager::~ChunkManager() {
//    
//    for (auto& kv : chunks) {
//        Chunk* chunk = kv.second;
//        if (chunk->isLoaded()) {
//            chunk->unload();
//        }
//        delete chunk;
//    }
//    chunks.clear();
//}
//
//Chunk* ChunkManager::createChunk(int cx, int cy) {
//   
//    Chunk* chunk = new Chunk(cx, cy);
//    return chunk;
//}
//
//void ChunkManager::updateChunks(const sf::Vector2f& playerPos, const sf::View& view) {
//    
//    int pcx = static_cast<int>(std::floor(playerPos.x / CHUNK_SIZE));
//    int pcy = static_cast<int>(std::floor(playerPos.y / CHUNK_SIZE));
//
//  
//    for (int dx = -LOAD_RADIUS; dx <= LOAD_RADIUS; ++dx) {
//        for (int dy = -LOAD_RADIUS; dy <= LOAD_RADIUS; ++dy) {
//            std::pair<int, int> coord = { pcx + dx, pcy + dy };
//            auto it = chunks.find(coord);
//            if (it == chunks.end()) {
//                
//                Chunk* newChunk = createChunk(coord.first, coord.second);
//                newChunk->load();
//                chunks[coord] = newChunk;
//            }
//            else {
//             
//                Chunk* existing = it->second;
//                if (!existing->isLoaded()) {
//                    existing->load();
//
//                }
//            }
//        }
//    }
//
//   
//    std::vector<std::pair<int, int>> toRemove;
//    for (auto& kv : chunks) {
//        int cx = kv.first.first;
//        int cy = kv.first.second;
//
//        if (std::abs(cx - pcx) > LOAD_RADIUS || std::abs(cy - pcy) > LOAD_RADIUS) {
//            Chunk* chunk = kv.second;
//            if (chunk->isLoaded()) {
//                chunk->unload();
//            }
//            delete chunk;
//            toRemove.push_back(kv.first);
//        }
//    }
//
//    for (auto& key : toRemove) {
//        chunks.erase(key);
//    }
//}
//
//void ChunkManager::draw(sf::RenderTarget& target, const sf::View& view) {
//
//    for (auto& kv : chunks) {
//        Chunk* chunk = kv.second;
//        if (chunk->isLoaded()) {
//            chunk->draw(target);
//        }
//    }
//}
//
//void ChunkManager::updateObjects(float dt, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) {
//
//    for (auto& kv : chunks) {
//        Chunk* chunk = kv.second;
//        if (chunk->isLoaded()) {
//            chunk->update(dt, blockedPolygons);
//        }
//    }
//}
//
//std::vector<sf::Vector2i> ChunkManager::getActiveChunkCoords() const {
//    std::vector<sf::Vector2i> active;
//    for (const auto& kv : chunks) {
//        if (kv.second->isLoaded()) {
//            active.push_back({ kv.first.first, kv.first.second });
//        }
//    }
//    return active;
//}
