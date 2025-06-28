#ifndef PATHFINDER_H
#define PATHFINDER_H

#include <vector>
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Rect.hpp> // Required for sf::FloatRect
#include <queue>
#include <map>
#include <cmath>
#include <limits>
#include "PathfindingGrid.h" 

struct Node {
    int gCost;
    int hCost;
    int fCost;
    Node* parent;
    sf::Vector2i position; // Grid coordinates

    Node(sf::Vector2i pos, Node* p = nullptr, int g = 0, int h = 0)
        : position(pos), parent(p), gCost(g), hCost(h) {
        fCost = gCost + hCost;
    }

    // For priority queue
    bool operator>(const Node& other) const {
        return fCost > other.fCost;
    }
};

class Pathfinder {
public:
    Pathfinder(const PathfindingGrid& grid); // Modified constructor
    ~Pathfinder();

    std::vector<sf::Vector2f> findPath(
        const sf::Vector2f& startPos,
        const sf::Vector2f& goalPos
    );

private:
    const PathfindingGrid& grid; // Reference to the preprocessed grid

    // Helper function for heuristic (Euclidean distance)
    int calculateHCost(const sf::Vector2i& current, const sf::Vector2i& goal);

    // Helper function to check if a cell is walkable (uses the grid)
    bool isWalkable(int x, int y);

    // Helper function to reconstruct the path (uses the grid)
    std::vector<sf::Vector2f> reconstructPath(Node* goalNode);
};

#endif // PATHFINDER_H