#include "PathfindingGrid.h"
#include <iostream> // For debug output, can be removed later

PathfindingGrid::PathfindingGrid(const sf::FloatRect& mapBounds, float gridSize)
    : mapOrigin(mapBounds.left, mapBounds.top),
    gridSize(gridSize),
    gridWidth(static_cast<int>(mapBounds.width / gridSize)),
    gridHeight(static_cast<int>(mapBounds.height / gridSize)) {

    if (gridSize <= 0) {
        // Handle error: gridSize must be positive
        std::cerr << "PathfindingGrid Error: gridSize must be positive." << std::endl;
        // Set to default valid values to prevent division by zero or negative sizes
        this->gridSize = 32.0f; // A default fallback
        this->gridWidth = static_cast<int>(mapBounds.width / this->gridSize);
        this->gridHeight = static_cast<int>(mapBounds.height / this->gridSize);
    }

    walkableGrid.resize(gridWidth * gridHeight, true); // Initialize all cells as walkable
    std::cout << "PathfindingGrid initialized: " << gridWidth << "x" << gridHeight << " cells, gridSize: " << gridSize << std::endl;
}

void PathfindingGrid::preprocess(const std::vector<std::vector<sf::Vector2f>>& staticObstacles) {
    std::cout << "PathfindingGrid: Starting preprocessing with " << staticObstacles.size() << " obstacles." << std::endl;
    for (int y = 0; y < gridHeight; ++y) {
        for (int x = 0; x < gridWidth; ++x) {
            // Sample the center and corners of the cell.  Using multiple sample
                // points reduces the chance that a thin obstacle will be missed when
                // only checking the center.
            sf::Vector2f cellCenter = gridToWorld({ x, y });
            float left = mapOrigin.x + static_cast<float>(x) * gridSize;
            float top = mapOrigin.y + static_cast<float>(y) * gridSize;
            std::array<sf::Vector2f, 5> points = {
                          cellCenter,
                          sf::Vector2f(left, top),
                          sf::Vector2f(left + gridSize, top),
                          sf::Vector2f(left, top + gridSize),
                          sf::Vector2f(left + gridSize, top + gridSize)
            };

            bool cellIsBlocked = false;
            for (const auto& p : points) {
                if (CollisionUtils::isInsideBlockedPolygon(p, staticObstacles)) {
                    cellIsBlocked = true;
                    break;
                }
            }

            if (cellIsBlocked) {
                walkableGrid[y * gridWidth + x] = false;
            }
            // For more robust checking, could also check corners of the cell or sample multiple points
            // For now, center check is a common approach.
        }
    }
    long walkableCount = 0;
    for (bool walkable : walkableGrid) {
        if (walkable) walkableCount++;
    }
    std::cout << "PathfindingGrid: Preprocessing complete. Walkable cells: " << walkableCount << "/" << (gridWidth * gridHeight) << std::endl;
}

bool PathfindingGrid::isCellWalkable(int x, int y) const {
    if (x < 0 || x >= gridWidth || y < 0 || y >= gridHeight) {
        return false; // Out of bounds
    }
    return walkableGrid[y * gridWidth + x];
}

sf::Vector2i PathfindingGrid::worldToGrid(const sf::Vector2f& worldPos) const {
    if (gridSize == 0) return { 0,0 }; // Should not happen if constructor validated
    int gridX = static_cast<int>((worldPos.x - mapOrigin.x) / gridSize);
    int gridY = static_cast<int>((worldPos.y - mapOrigin.y) / gridSize);
    return { gridX, gridY };
}

sf::Vector2f PathfindingGrid::gridToWorld(const sf::Vector2i& gridPos) const {
    float worldX = mapOrigin.x + (static_cast<float>(gridPos.x) + 0.5f) * gridSize;
    float worldY = mapOrigin.y + (static_cast<float>(gridPos.y) + 0.5f) * gridSize;
    return { worldX, worldY };
}

int PathfindingGrid::getWidth() const
{
    return gridWidth;
}

int PathfindingGrid::getHeight() const
{
    return gridHeight;
}

float PathfindingGrid::getGridSize() const
{
    return gridSize;
}

const sf::Vector2f& PathfindingGrid::getMapOrigin() const
{
    return mapOrigin;
}
