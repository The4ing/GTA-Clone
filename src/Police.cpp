#include "Police.h"
#include "ResourceManager.h"
#include <cmath>
#include <algorithm>
#include <iostream>
#include "CollisionUtils.h"

// MAP_BOUNDS should be defined where it's used or passed appropriately.
// For now, using the one defined in Police.h or a local one if needed.
// If it's a global constant, it should be in a Constants.h or similar.
const sf::FloatRect MAP_BOUNDS(0.f, 0.f, 4640.f, 4670.f); // Assuming this is the correct map bounds

Police::Police(sf::Vector2f target) : targetPos(target) {
    sprite.setTexture(ResourceManager::getInstance().getTexture("police"));
    framesPerRow = 6;
    frameWidth = sprite.getTexture()->getSize().x / framesPerRow;
    frameHeight = sprite.getTexture()->getSize().y;
    sprite.setTextureRect({ 0, 0, frameWidth, frameHeight });
    sprite.setOrigin(frameWidth / 2.f, frameHeight / 2.f);
    sprite.setPosition(100, 100); // Initial position
    setRandomWanderDestination(MAP_BOUNDS);

    sprite.setScale(0.07f, 0.07f);
    speed = 60.f; // Slightly increased speed for testing path following
    currentFrame = 0;

    // Initialize pathfinding members
    currentPathIndex = 0;
    repathTimer = 0.f;
    pathFailCooldown = 0.f; // Initialize this as well
}

void Police::update(float dt, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) {
    repathTimer += dt;
    if (pathFailCooldown > 0.f) {
        pathFailCooldown -= dt;
    }

    float distToPlayer = std::hypot(targetPos.x - getPosition().x, targetPos.y - getPosition().y);

    // State transition logic
    if (distToPlayer <= detectionRadius) {
        if (state != PoliceState::Chasing) {
            state = PoliceState::Chasing;
            currentFrame = 0; // Reset animation
            currentPath.clear(); // Clear previous path
            currentPathIndex = 0;
            repathTimer = 1.0f; // Force repath quickly when switching to chase
            std::cout << "Police: Switched to Chasing state." << std::endl;
        }
    } else if (state == PoliceState::Chasing && distToPlayer > detectionRadius + 30.f) { // Add hysteresis
        state = PoliceState::Idle;
        currentFrame = 0; // Reset animation
        currentPath.clear(); // Clear previous path
        currentPathIndex = 0;
        setRandomWanderDestination(MAP_BOUNDS); // Get a new wander destination
        std::cout << "Police: Switched to Idle state." << std::endl;
    }

    // Pathfinding and Destination Logic
    if (state == PoliceState::Chasing) {
        // Condition for finding a new path to the player
        if (pathFailCooldown <= 0.f && (currentPath.empty() || currentPathIndex >= currentPath.size() || repathTimer > 1.0f)) {
            std::cout << "Police (Chasing): Attempting to find path to player at (" << targetPos.x << ", " << targetPos.y << ")" << std::endl;
            currentPath = pathfinder.findPath(getPosition(), targetPos, blockedPolygons, MAP_BOUNDS, PATHFINDING_GRID_SIZE);
            std::cout << "Police (Chasing): Path found with size: " << currentPath.size() << std::endl;
            currentPathIndex = 0;
            repathTimer = 0.f;
            if (currentPath.empty()) {
                std::cerr << "Police (Chasing): Failed to find path to player." << std::endl;
                pathFailCooldown = 3.0f; // Wait before trying again
            }
        }
    } else if (state == PoliceState::Idle) {
        // Condition for finding a new path to the wander destination
        if (currentPath.empty() || currentPathIndex >= currentPath.size()) {
             // If path is finished or empty, and we are idling, find path to current wander destination
            if (pathFailCooldown <= 0.f) {
                 std::cout << "Police (Idle): Attempting to find path to wander destination (" << wanderDestination.x << ", " << wanderDestination.y << ")" << std::endl;
                currentPath = pathfinder.findPath(getPosition(), wanderDestination, blockedPolygons, MAP_BOUNDS, PATHFINDING_GRID_SIZE);
                std::cout << "Police (Idle): Path found with size: " << currentPath.size() << std::endl;
                currentPathIndex = 0;
                repathTimer = 0.f; // Reset timer as we just pathed
                if (currentPath.empty()) {
                    std::cerr << "Police (Idle): Failed to find path to wander destination. Setting new one." << std::endl;
                    pathFailCooldown = 1.0f; // Wait a bit before trying new random spot
                    setRandomWanderDestination(MAP_BOUNDS); // Get a new random spot immediately
                }
            }
        }
    }

    // Movement Execution along the path
    if (!currentPath.empty() && currentPathIndex < currentPath.size()) {
        sf::Vector2f nextWaypoint = currentPath[currentPathIndex];
        moveToward(nextWaypoint, dt, blockedPolygons); // moveToward now aims for the waypoint

        float distanceToWaypoint = std::hypot(nextWaypoint.x - getPosition().x, nextWaypoint.y - getPosition().y);
        // Check if waypoint is reached
        if (distanceToWaypoint < PATHFINDING_GRID_SIZE / 2.0f) { // Threshold to consider waypoint reached
            currentPathIndex++;
            //std::cout << "Police: Reached waypoint. Moving to index " << currentPathIndex << "/" << currentPath.size() << std::endl;
            if (currentPathIndex >= currentPath.size()) {
                //std::cout << "Police: Path completed." << std::endl;
                currentPath.clear();
                currentPathIndex = 0;
                if (state == PoliceState::Idle) {
                    //std::cout << "Police (Idle): Path to wander destination completed. Setting new random destination." << std::endl;
                    setRandomWanderDestination(MAP_BOUNDS); // Path completed, get new wander target
                }
            }
        }
    } else if (state == PoliceState::Chasing && pathFailCooldown > 0.f) {
        // Chasing but path failed, do nothing or a fallback (e.g., simple move, or just wait)
        // For now, just waits for pathFailCooldown to expire.
    } else if (state == PoliceState::Idle && currentPath.empty() && pathFailCooldown <=0.f) {
        // Idle, current path is empty (e.g. first time, or previous path failed/completed)
        // and not in cooldown from a recent path failure
        //std::cout << "Police (Idle): Current path empty and not in cooldown. Setting new random destination." << std::endl;
        setRandomWanderDestination(MAP_BOUNDS); // This will trigger path calculation in the next update cycle.
    }


    // Animation Update (remains largely the same)
    animationTimer += dt;
    if (animationTimer >= animationSpeed) {
        animationTimer = 0.f;
        // Basic check if moving: if currentPath is not empty and we are not at the end.
        bool isMoving = !currentPath.empty() && currentPathIndex < currentPath.size();
        if (isMoving) {
            currentFrame = (currentFrame + 1) % framesPerRow;
        } else {
            currentFrame = 0; // Idle frame if not moving
        }
    }
    sprite.setTextureRect({
        currentFrame * frameWidth,
        0, // Assuming police animation is single row
        frameWidth,
        frameHeight
    });
}

void Police::moveToward(const sf::Vector2f& target, float dt, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) {
    sf::Vector2f dir = target - getPosition();
    float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);

    if (len < 1.0f) { // Already at or very close to the target waypoint
        return;
    }

    dir /= len; // Normalize

    float angle = std::atan2(dir.y, dir.x) * 180.f / static_cast<float>(M_PI); // Ensure M_PI is float if dir.y, dir.x are
    sprite.setRotation(angle + 90.f); // Adjusted rotation based on typical sprite orientation (e.g. top-down sprite facing up)
                                      // Original was angle - 270.f. If sprite faces right, use angle. If up, angle + 90. etc.

    sf::Vector2f nextPos = sprite.getPosition() + dir * speed * dt;

    // Local collision check (optional, pathfinder should avoid major obstacles)
    // This can be a simpler check or removed if pathfinding is robust enough
    // For now, let's keep it to avoid minor overlaps or dynamic entities not in pathfinding grid
    bool collision = false;
    // Simple bounding box for nextPos for this local check (not full radius, just point)
    for (const auto& poly : blockedPolygons) {
        if (CollisionUtils::pointInPolygon(nextPos, poly)) {
            //std::cout << "Police: moveToward detected local collision." << std::endl;
            collision = true;
            break;
        }
    }

    if (!collision) {
        sprite.move(dir * speed * dt);
    }
}

void Police::draw(sf::RenderWindow& window) {
    window.draw(sprite);

    // Optional: Draw current path for debugging
    //if (!currentPath.empty()) {
    //    sf::VertexArray lines(sf::LinesStrip);
    //    lines.append(sf::Vertex(getPosition(), sf::Color::Blue)); // Line from police to first waypoint
    //    for (size_t i = currentPathIndex; i < currentPath.size(); ++i) {
    //        lines.append(sf::Vertex(currentPath[i], sf::Color::Blue));
    //    }
    //    window.draw(lines);
    //}
}

void Police::takeDamage(int amount) {
    health -= amount;
    if (health < 0) health = 0;
    //std::cout << "Police: Took " << amount << " damage, health is now " << health << std::endl;
}

bool Police::isDead() const {
    return health <= 0;
}

float Police::getCollisionRadius() const {
    // Effective radius after scaling. Original frameWidth * scale / 2
    return (frameWidth * sprite.getScale().x) / 2.0f * 0.8f; // 0.8f is an adjustment factor
}

void Police::setTargetPosition(const sf::Vector2f& pos) {
    if (targetPos != pos) {
        targetPos = pos;
        // Optionally, force a repath if the target moves significantly
        // repathTimer = 1.0f; // Or some other logic
        // currentPath.clear(); // If player moves, old path is invalid
        // currentPathIndex = 0;
    }
}

void Police::setRandomWanderDestination(const sf::FloatRect& mapBounds) {
    sf::Vector2f candidate;
    bool found = false;
    int tries = 0;
    const int MAX_TRIES = 20;

    // Try to find a point that is somewhat distant and within map bounds
    do {
        float angle = static_cast<float>(rand()) / RAND_MAX * 2.f * static_cast<float>(M_PI);
        float radius = 100.f + static_cast<float>(rand()) / RAND_MAX * 400.f; // Wander between 100 and 500 units away
        candidate = getPosition() + sf::Vector2f(std::cos(angle), std::sin(angle)) * radius;

        if (mapBounds.contains(candidate)) {
            // Optional: Could add a check here to see if candidate is inside a blockedPolygon, though pathfinder should handle it.
            // For simplicity, we assume pathfinder will find a way or fail gracefully.
            found = true;
        }
        tries++;
    } while (!found && tries < MAX_TRIES);

    if (found) {
        wanderDestination = candidate;
    } else {
        // Fallback: if many tries fail, pick a point towards center or a default safe spot
        wanderDestination = sf::Vector2f(mapBounds.width / 2.f, mapBounds.height / 2.f);
    }
    //std::cout << "Police (Idle): New wander destination set to (" << wanderDestination.x << ", " << wanderDestination.y << ")" << std::endl;
    currentPath.clear();      // Clear old path
    currentPathIndex = 0;     // Reset index
    repathTimer = 0.f;        // Allow immediate repathing for new wander destination
    pathFailCooldown = 0.f;   // Reset path fail cooldown
}

// Ensure M_PI is defined if not available (e.g. in MSVC)
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
