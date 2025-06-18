#include "Pathfinder.h"      // Adjust path as necessary, assumes Pathfinder.h is in include path
#include "CollisionUtils.h"  // Adjust path as necessary, assumes CollisionUtils.h is in include path
#include <vector>
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <iostream>
#include <cassert>
#include <cmath> // For std::hypot

// Helper function to print the path (for debugging)
void printPath(const std::vector<sf::Vector2f>& path) {
    std::cout << "Path (size " << path.size() << "): ";
    for (const auto& p : path) {
        std::cout << "(" << p.x << "," << p.y << ") ";
    }
    std::cout << std::endl;
}

// Helper function to check if a point is inside any of the polygons
bool isPointInPolygons(const sf::Vector2f& point, const std::vector<std::vector<sf::Vector2f>>& polygons) {
    for (const auto& polygon : polygons) {
        if (CollisionUtils::pointInPolygon(point, polygon)) {
            return true;
        }
    }
    return false;
}

// Helper to check if two sf::Vector2f are close enough
bool arePointsClose(const sf::Vector2f& p1, const sf::Vector2f& p2, float tolerance = 1.0f) {
    return std::hypot(p1.x - p2.x, p1.y - p2.y) < tolerance;
}

// Test Case 1: Simple Straight Path (No Obstacles)
void testSimpleStraightPath() {
    std::cout << "--- Test Case 1: Simple Straight Path ---" << std::endl;
    Pathfinder pathfinder;
    sf::FloatRect mapBounds(0.f, 0.f, 300.f, 300.f);
    float gridSize = 10.0f;
    std::vector<std::vector<sf::Vector2f>> obstacles; // No obstacles
    sf::Vector2f startPos(50.f, 50.f);
    sf::Vector2f goalPos(200.f, 50.f);

    std::vector<sf::Vector2f> path = pathfinder.findPath(startPos, goalPos, obstacles, mapBounds, gridSize);
    printPath(path);

    assert(!path.empty() && "Test 1 Failed: Path should not be empty.");
    if (!path.empty()) {
        // Pathfinder returns centers of cells. Allow gridSize/2 tolerance.
        assert(arePointsClose(path.front(), startPos, gridSize) && "Test 1 Failed: Path start is not close to startPos.");
        assert(arePointsClose(path.back(), goalPos, gridSize) && "Test 1 Failed: Path end is not close to goalPos.");
        for (const auto& p : path) {
            assert(mapBounds.contains(p) && "Test 1 Failed: Point in path is outside mapBounds.");
            assert(!isPointInPolygons(p, obstacles) && "Test 1 Failed: Point in path is inside an obstacle (should be none).");
        }
    }
    std::cout << "Test Case 1 Passed." << std::endl;
}

// Test Case 2: Path Blocked by Single Rectangular Obstacle
void testPathAroundObstacle() {
    std::cout << "\n--- Test Case 2: Path Blocked by Single Rectangular Obstacle ---" << std::endl;
    Pathfinder pathfinder;
    sf::FloatRect mapBounds(0.f, 0.f, 300.f, 300.f);
    float gridSize = 10.0f;
    sf::Vector2f startPos(50.f, 150.f);
    sf::Vector2f goalPos(250.f, 150.f);

    // Obstacle between start and goal
    std::vector<std::vector<sf::Vector2f>> obstacles;
    std::vector<sf::Vector2f> rectObstacle;
    rectObstacle.push_back(sf::Vector2f(140.f, 130.f)); // Top-left
    rectObstacle.push_back(sf::Vector2f(160.f, 130.f)); // Top-right
    rectObstacle.push_back(sf::Vector2f(160.f, 170.f)); // Bottom-right
    rectObstacle.push_back(sf::Vector2f(140.f, 170.f)); // Bottom-left
    obstacles.push_back(rectObstacle);

    std::vector<sf::Vector2f> path = pathfinder.findPath(startPos, goalPos, obstacles, mapBounds, gridSize);
    printPath(path);

    assert(!path.empty() && "Test 2 Failed: Path should not be empty.");
    if (!path.empty()) {
        assert(arePointsClose(path.front(), startPos, gridSize) && "Test 2 Failed: Path start is not close to startPos.");
        assert(arePointsClose(path.back(), goalPos, gridSize) && "Test 2 Failed: Path end is not close to goalPos.");
        bool pathIntersectsObstacle = false;
        for (const auto& p : path) {
            assert(mapBounds.contains(p) && "Test 2 Failed: Point in path is outside mapBounds.");
            if (isPointInPolygons(p, obstacles)) {
                // Allow points to be very close to edge, but not *inside* center of cell sense
                // This check is tricky because path points are cell centers.
                // A cell center might be "in" a polygon if the polygon edge cuts through the cell.
                // A more robust check would be if the segment between path points intersects obstacle.
                // For now, let's check if the path *clearly* goes around.
                // A simple check is that not *all* points are on the direct line.
                // If the path has points with y != startPos.y (for a horizontal expected path)
                // it means it went around.
            }
        }
        // Check if path deviates (goes around)
        bool deviated = false;
        for(const auto& p : path) {
            if (std::abs(p.y - startPos.y) > gridSize/1.5f) { // Check if y changed significantly
                deviated = true;
                break;
            }
        }
        assert(deviated && "Test 2 Failed: Path did not deviate to go around the obstacle.");
         // Stronger check: ensure no *segment* of the path crosses the obstacle.
        // This is more complex. For now, checking points is a first step.
        // The individual points (cell centers) should not be inside the polygon.
        for(const auto& p_cell_center : path) {
             assert(!CollisionUtils::pointInPolygon(p_cell_center, rectObstacle) && "Test 2 Failed: A path cell center is inside the obstacle.");
        }
    }
    std::cout << "Test Case 2 Passed." << std::endl;
}

// Test Case 3: No Path Possible (Goal Enclosed)
void testNoPathPossible() {
    std::cout << "\n--- Test Case 3: No Path Possible (Goal Enclosed) ---" << std::endl;
    Pathfinder pathfinder;
    sf::FloatRect mapBounds(0.f, 0.f, 300.f, 300.f);
    float gridSize = 10.0f;
    sf::Vector2f startPos(50.f, 50.f);
    sf::Vector2f goalPos(150.f, 150.f); // Goal inside the box

    std::vector<std::vector<sf::Vector2f>> obstacles;
    std::vector<sf::Vector2f> box;
    // Box around (150,150)
    box.push_back(sf::Vector2f(120.f, 120.f)); // Top-left
    box.push_back(sf::Vector2f(180.f, 120.f)); // Top-right
    box.push_back(sf::Vector2f(180.f, 180.f)); // Bottom-right
    box.push_back(sf::Vector2f(120.f, 180.f)); // Bottom-left
    obstacles.push_back(box);

    std::vector<sf::Vector2f> path = pathfinder.findPath(startPos, goalPos, obstacles, mapBounds, gridSize);
    printPath(path);

    assert(path.empty() && "Test 3 Failed: Path should be empty as goal is enclosed.");
    std::cout << "Test Case 3 Passed." << std::endl;
}

// Test Case 4: Start or Goal Outside Map Bounds (or non-walkable)
void testStartGoalOutsideBounds() {
    std::cout << "\n--- Test Case 4: Start or Goal Outside Map Bounds ---" << std::endl;
    Pathfinder pathfinder;
    sf::FloatRect mapBounds(0.f, 0.f, 100.f, 100.f); // Small map
    float gridSize = 10.0f;
    std::vector<std::vector<sf::Vector2f>> obstacles;

    sf::Vector2f startPos_valid(50.f, 50.f);
    sf::Vector2f goalPos_invalid(150.f, 150.f); // Outside mapBounds

    std::vector<sf::Vector2f> path = pathfinder.findPath(startPos_valid, goalPos_invalid, obstacles, mapBounds, gridSize);
    printPath(path);
    assert(path.empty() && "Test 4a Failed: Path should be empty if goal is outside map bounds.");
    std::cout << "Test Case 4a (Goal outside) Passed." << std::endl;

    sf::Vector2f startPos_invalid(-10.f, 50.f); // Outside mapBounds
    sf::Vector2f goalPos_valid(80.f, 80.f);

    path = pathfinder.findPath(startPos_invalid, goalPos_valid, obstacles, mapBounds, gridSize);
    printPath(path);
    assert(path.empty() && "Test 4b Failed: Path should be empty if start is outside map bounds.");
    std::cout << "Test Case 4b (Start outside) Passed." << std::endl;
}

// Test Case 5: Start and Goal are the same
void testStartEqualsGoal() {
    std::cout << "\n--- Test Case 5: Start and Goal are the same ---" << std::endl;
    Pathfinder pathfinder;
    sf::FloatRect mapBounds(0.f, 0.f, 300.f, 300.f);
    float gridSize = 10.0f;
    std::vector<std::vector<sf::Vector2f>> obstacles;
    sf::Vector2f startPos(50.f, 50.f);
    sf::Vector2f goalPos(50.f, 50.f); // Same as start

    std::vector<sf::Vector2f> path = pathfinder.findPath(startPos, goalPos, obstacles, mapBounds, gridSize);
    printPath(path);

    assert(!path.empty() && "Test 5 Failed: Path should not be empty.");
    // Path should contain just the goal/start position
    assert(path.size() == 1 && "Test 5 Failed: Path should contain exactly one point.");
    if (!path.empty()) {
         assert(arePointsClose(path.front(), startPos, gridSize / 2.0f ) && "Test 5 Failed: Path point is not close to start/goal.");
    }
    std::cout << "Test Case 5 Passed." << std::endl;
}


int main() {
    std::cout << "Starting Pathfinder Tests..." << std::endl;

    // Mock CollisionUtils for tests if it's not available or has dependencies.
    // For this test, we assume CollisionUtils::pointInPolygon is linkable and works.
    // If not, these tests would need a mock or a simpler line-segment intersection for obstacles.

    testSimpleStraightPath();
    testPathAroundObstacle();
    testNoPathPossible();
    testStartGoalOutsideBounds();
    testStartEqualsGoal();

    std::cout << "\nAll Pathfinder Tests Completed." << std::endl;
    return 0;
}
