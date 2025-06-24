#include "Pathfinder.h"
#include "CollisionUtils.h" // Assuming this file exists and is in the include path
#include <iostream> // For debugging
#include <algorithm> // For std::reverse

// Node comparison structure for priority queue
struct CompareNodeFScore {
    bool operator()(const Node* a, const Node* b) const {
        // Min-heap: smallest fCost has higher priority
        if (a->fCost == b->fCost) {
            return a->hCost > b->hCost; // Tie-breaker: prefer smaller hCost
        }
        return a->fCost > b->fCost;
    }
};

Pathfinder::Pathfinder() {
    // Constructor implementation (if needed)
}

Pathfinder::~Pathfinder() {
    // Destructor implementation (if needed)
}

int Pathfinder::calculateHCost(const sf::Vector2i& current, const sf::Vector2i& goal) {
    // Euclidean distance heuristic (scaled by 10 for integer arithmetic to avoid floats in costs)
    int dx = goal.x - current.x;
    int dy = goal.y - current.y;
    // Using int for cost, common practice in grid-based A*
    return static_cast<int>(std::sqrt(dx * dx + dy * dy) * 10.0f);
}

bool Pathfinder::isWalkable(
    int x,
    int y,
    int gridWidth,
    int gridHeight,
    const std::vector<std::vector<sf::Vector2f>>& obstacles,
    float gridSize,
    const sf::Vector2f& mapOrigin) {

    // Check bounds: Ensure the grid cell (x, y) is within the map's grid dimensions.
    if (x < 0 || x >= gridWidth || y < 0 || y >= gridHeight) {
        return false;
    }

    // Convert grid coordinates to world coordinates (center of the cell)
    // This is necessary to check against world-space obstacles.
    sf::Vector2f worldPos(
        mapOrigin.x + (static_cast<float>(x) + 0.5f) * gridSize,
        mapOrigin.y + (static_cast<float>(y) + 0.5f) * gridSize
    );

    // Check for collisions with obstacles
    for (const auto& obstacle : obstacles) {
        if (CollisionUtils::pointInPolygon(worldPos, obstacle)) {
            return false; // Point is inside an obstacle
        }
    }

    return true; // Walkable if not out of bounds and not in an obstacle
}

std::vector<sf::Vector2f> Pathfinder::reconstructPath(Node* goalNode, float gridSize, const sf::Vector2f& mapOrigin) {
    std::vector<sf::Vector2f> path;
    Node* current = goalNode;
    while (current != nullptr) {
        // Convert grid coordinates back to world coordinates (center of the cell)
        sf::Vector2f worldPos(
            mapOrigin.x + (static_cast<float>(current->position.x) + 0.5f) * gridSize,
            mapOrigin.y + (static_cast<float>(current->position.y) + 0.5f) * gridSize
        );
        path.push_back(worldPos);
        current = current->parent; // Move to the parent node
    }
    // The path is constructed backwards from goal to start, so it needs to be reversed.
    std::reverse(path.begin(), path.end());
    return path;
}

std::vector<sf::Vector2f> Pathfinder::findPath(
    const sf::Vector2f& startPos,
    const sf::Vector2f& goalPos,
    const std::vector<std::vector<sf::Vector2f>>& obstacles,
    const sf::FloatRect& mapBounds,
    float gridSize) {

    std::vector<sf::Vector2f> worldPath; // The final path in world coordinates

    // --- Grid Setup ---
    sf::Vector2f mapOrigin(mapBounds.left, mapBounds.top);
    int gridWidth = static_cast<int>(mapBounds.width / gridSize);
    int gridHeight = static_cast<int>(mapBounds.height / gridSize);

    // Convert start and goal positions from world coordinates to grid coordinates
    sf::Vector2i startNodePos(
        static_cast<int>((startPos.x - mapOrigin.x) / gridSize),
        static_cast<int>((startPos.y - mapOrigin.y) / gridSize)
    );
    sf::Vector2i goalNodePos(
        static_cast<int>((goalPos.x - mapOrigin.x) / gridSize),
        static_cast<int>((goalPos.y - mapOrigin.y) / gridSize)
    );

    // Basic validation for start/goal positions
    if (!isWalkable(startNodePos.x, startNodePos.y, gridWidth, gridHeight, obstacles, gridSize, mapOrigin)) {
      //  std::cerr << "Pathfinder Error: Start position (" << startPos.x << "," << startPos.y << ") mapped to grid (" << startNodePos.x << "," << startNodePos.y << ") is not walkable or out of bounds." << std::endl;
        return worldPath; // Return empty path
    }
    if (!isWalkable(goalNodePos.x, goalNodePos.y, gridWidth, gridHeight, obstacles, gridSize, mapOrigin)) {
    //    std::cerr << "Pathfinder Error: Goal position (" << goalPos.x << "," << goalPos.y << ") mapped to grid (" << goalNodePos.x << "," << goalNodePos.y << ") is not walkable or out of bounds." << std::endl;
        return worldPath; // Return empty path
    }
    if (startNodePos == goalNodePos) {
        worldPath.push_back(goalPos); // Path is just the goal itself if start and goal are the same
        return worldPath;
    }

    // --- A* Algorithm Initialization ---
    // Open list: nodes to be evaluated, ordered by fCost.
    // Uses a min-priority queue to efficiently get the node with the lowest fCost.
    std::priority_queue<Node*, std::vector<Node*>, CompareNodeFScore> openList;

    // Closed list: stores coordinates of nodes already evaluated to prevent reprocessing.
    // A map from grid coordinates (pair<int,int>) to boolean (true if closed).
    std::map<std::pair<int, int>, bool> closedList;

    // Open set map: stores coordinates of nodes currently in the openList for quick lookup.
    // This helps in checking if a neighbor is already in the openList and potentially updating it.
    // Maps grid coordinates to the Node pointer in the openList.
    std::map<std::pair<int, int>, Node*> openSetMap;

    // Store all allocated nodes to manage their memory.
    // Nodes are dynamically allocated and need to be deleted at the end.
    std::vector<Node*> allNodes;

    // Create start node
    Node* startNode = new Node(startNodePos, nullptr, 0, calculateHCost(startNodePos, goalNodePos));
    openList.push(startNode);
    openSetMap[{startNodePos.x, startNodePos.y}] = startNode;
    allNodes.push_back(startNode);


    // --- A* Main Loop ---
    while (!openList.empty()) {
        Node* currentNode = openList.top(); // Get node with the lowest fCost
        openList.pop(); // Remove from open list

        // Remove from openSetMap as it's now being processed
        openSetMap.erase({ currentNode->position.x, currentNode->position.y });

        // Check if we reached the goal
        if (currentNode->position == goalNodePos) {
            worldPath = reconstructPath(currentNode, gridSize, mapOrigin);
            break; // Path found
        }

        // Add current node to closed list (it has been processed)
        closedList[{currentNode->position.x, currentNode->position.y}] = true;

        // Explore neighbors (8 directions: N, S, E, W, NE, NW, SE, SW)
        for (int dx = -1; dx <= 1; ++dx) {
            for (int dy = -1; dy <= 1; ++dy) {
                if (dx == 0 && dy == 0) continue; // Skip current node itself

                sf::Vector2i neighborPos(currentNode->position.x + dx, currentNode->position.y + dy);

                // Check if neighbor is walkable
                if (!isWalkable(neighborPos.x, neighborPos.y, gridWidth, gridHeight, obstacles, gridSize, mapOrigin)) {
                    continue; // Skip unwalkable neighbors
                }

                // Check if neighbor is in closed list
                if (closedList.count({ neighborPos.x, neighborPos.y })) {
                    continue; // Skip already processed neighbors
                }

                // Calculate tentative gCost for the path from start to this neighbor through currentNode
                // Cost: 10 for cardinal (horizontal/vertical), 14 for diagonal (approx. sqrt(2)*10)
                int moveCost = ((dx != 0 && dy != 0) ? 14 : 10);
                int tentativeGCost = currentNode->gCost + moveCost;

                // Check if neighbor is already in the open list
                Node* neighborNode = nullptr;
                if (openSetMap.count({ neighborPos.x, neighborPos.y })) {
                    neighborNode = openSetMap[{neighborPos.x, neighborPos.y}];

                    // If new path to neighbor is worse or same, skip
                    if (tentativeGCost >= neighborNode->gCost) {
                        continue;
                    }
                }
                else {
                    // Neighbor is not in open list yet, create a new node
                    neighborNode = new Node(neighborPos);
                    allNodes.push_back(neighborNode); // Add to memory management list
                    // HCost is calculated once for a new node
                    neighborNode->hCost = calculateHCost(neighborPos, goalNodePos);
                }

                // This path is better or neighborNode is new: update its details
                neighborNode->parent = currentNode;
                neighborNode->gCost = tentativeGCost;
                neighborNode->fCost = neighborNode->gCost + neighborNode->hCost;

                // If it was not in openSetMap, add it.
                // If it was already in openSetMap, its costs are updated.
                // The priority queue doesn't directly support decreasing a key.
                // So, we add the (potentially updated) node again. The old instance
                // (if any) will eventually be popped and ignored due to being in closedList
                // or having a higher fCost. This is a common way to handle updates with std::priority_queue.
                // The CompareNodeFScore will ensure the one with better fCost is processed first.
                if (!openSetMap.count({ neighborPos.x, neighborPos.y })) {
                    openList.push(neighborNode);
                    openSetMap[{neighborPos.x, neighborPos.y}] = neighborNode;
                }
                else {
                    // To make sure priority queue reorders if gCost was updated leading to a better fCost:
                    // A bit of a hack for std::priority_queue: remove and re-add.
                    // This is inefficient. A better PQ implementation would offer decrease-key.
                    // For now, pushing a new entry is simpler and often works well enough.
                    // The old entry will be processed later but likely skipped if already in closed list.
                    // To truly update, one would need to build a priority queue that supports it.
                    // Given the current structure, we're pushing and relying on the closed list.
                    // Let's ensure it's in the openList if it was updated
                    // (if it was already there and updated, it's already in openList via its pointer)
                    // Forcing a re-add to potentially re-prioritize:
                    openList.push(neighborNode);
                    // No need to re-add to openSetMap if it was already there, map holds the pointer.
                }
            }
        }
    }

    // --- Cleanup ---
    // Delete all dynamically allocated nodes
    for (Node* node : allNodes) {
        delete node;
    }
    allNodes.clear();

    if (worldPath.empty() && !(startNodePos == goalNodePos)) { // Avoid message if start was goal
        std::cerr << "Pathfinder: No path found from (" << startPos.x << "," << startPos.y << ") to (" << goalPos.x << "," << goalPos.y << ")." << std::endl;
    }

    return worldPath;
}