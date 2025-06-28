#ifndef PATHFINDING_GRID_H
#define PATHFINDING_GRID_H

#include <vector>
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Rect.hpp>
#include "CollisionUtils.h" // For pointInPolygon

class PathfindingGrid {
public:
    PathfindingGrid(const sf::FloatRect& mapBounds, float gridSize);

    // Preprocesses the grid based on static collision polygons.
    // Should be called once at game load time.
    void preprocess(const std::vector<std::vector<sf::Vector2f>>& staticObstacles);

    // Checks if a specific grid cell is walkable.
    bool isCellWalkable(int x, int y) const;

    // Converts world coordinates to grid coordinates.
    sf::Vector2i worldToGrid(const sf::Vector2f& worldPos) const;

    // Converts grid coordinates to world coordinates (center of the cell).
    sf::Vector2f gridToWorld(const sf::Vector2i& gridPos) const;

    int getWidth() const { return gridWidth; }
    int getHeight() const { return gridHeight; }
    float getGridSize() const { return gridSize; }
    const sf::Vector2f& getMapOrigin() const { return mapOrigin; }


private:
    sf::Vector2f mapOrigin;
    float gridSize;
    int gridWidth;
    int gridHeight;
    std::vector<bool> walkableGrid; // Flattened 2D grid: index = y * gridWidth + x
};

#endif // PATHFINDING_GRID_H