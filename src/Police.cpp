#include "Police.h"
#include "ResourceManager.h"
#include <cmath>
#include <algorithm>
#include <iostream>
#include "CollisionUtils.h"



Police::Police(sf::Vector2f target) :targetPos(target) {
    sprite.setTexture(ResourceManager::getInstance().getTexture("police"));
    framesPerRow = 6;
    frameWidth = sprite.getTexture()->getSize().x / framesPerRow;
    frameHeight = sprite.getTexture()->getSize().y;
    sprite.setTextureRect({ 0, 0, frameWidth, frameHeight });
    sprite.setOrigin(frameWidth / 2.f, frameHeight / 2.f);
    //sprite.setPosition(target);
    sprite.setPosition(100, 100);
    setRandomWanderDestination(MAP_BOUNDS);

     sprite.setScale(0.07f, 0.07f); 
    speed = 40.f;
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
    }
    else if (state == PoliceState::Chasing && distToPlayer > detectionRadius + 30.f) { // Add hysteresis
        state = PoliceState::Idle;
        currentFrame = 0; // Reset animation
        currentPath.clear(); // Clear previous path
        currentPathIndex = 0;
        setRandomWanderDestination(MAP_BOUNDS); // Get a new wander destination
        std::cout << "Police: Switched to Idle state." << std::endl;
    }

    // Backing up state: move backwards smoothly
    if (state == PoliceState::BackingUp) {
        float stepSize = speed * dt;
        if (backedUpSoFar < backUpDistance) {
            sf::Vector2f currentPos = getPosition();
            sf::Vector2f nextPos = currentPos + backUpDirection * stepSize;

            if (checkCollision(currentPos, nextPos, blockedPolygons, getCollisionRadius())) {
                // Stuck even going back, reset path and pick new destination
                currentPath.clear();
                currentPathIndex = 0;
                setRandomWanderDestination(MAP_BOUNDS);
                state = PoliceState::Idle;
                backedUpSoFar = 0.f;
                return;
            }

            sprite.setPosition(nextPos);
            backedUpSoFar += stepSize;

            float angle = std::atan2(backUpDirection.y, backUpDirection.x) * 180.f / M_PI;
            sprite.setRotation(angle - 270.f);
            return; // Do not continue normal movement while backing up
        }
        else {
            // Finished backing up, reset and go idle
            backedUpSoFar = 0.f;
            state = PoliceState::Idle;
            currentPath.clear();
            currentPathIndex = 0;
            setRandomWanderDestination(MAP_BOUNDS);
            return;
        }
    }

    // Pathfinding and Destination Logic
    if (state == PoliceState::Chasing) {
        if (pathFailCooldown <= 0.f && (currentPath.empty() || currentPathIndex >= currentPath.size() || repathTimer > 1.0f)) {
            std::cout << "Police (Chasing): Attempting to find path to player at (" << targetPos.x << ", " << targetPos.y << ")" << std::endl;
            currentPath = pathfinder.findPath(getPosition(), targetPos, blockedPolygons, MAP_BOUNDS, PATHFINDING_GRID_SIZE);
            std::cout << "Police (Chasing): Path found with size: " << currentPath.size() << std::endl;
            currentPathIndex = 0;
            repathTimer = 0.f;
            if (currentPath.empty()) {
                std::cerr << "Police (Chasing): Failed to find path to player." << std::endl;
                pathFailCooldown = 3.0f;
            }
        }
    }
    else if (state == PoliceState::Idle) {
        if (currentPath.empty() || currentPathIndex >= currentPath.size()) {
            if (pathFailCooldown <= 0.f) {
                std::cout << "Police (Idle): Attempting to find path to wander destination (" << wanderDestination.x << ", " << wanderDestination.y << ")" << std::endl;
                currentPath = pathfinder.findPath(getPosition(), wanderDestination, blockedPolygons, MAP_BOUNDS, PATHFINDING_GRID_SIZE);
                std::cout << "Police (Idle): Path found with size: " << currentPath.size() << std::endl;
                currentPathIndex = 0;
                repathTimer = 0.f;
                if (currentPath.empty()) {
                    std::cerr << "Police (Idle): Failed to find path to wander destination. Setting new one." << std::endl;
                    pathFailCooldown = 1.0f;
                    setRandomWanderDestination(MAP_BOUNDS);
                }
            }
        }
    }

    // Movement Execution along the path
    if (!currentPath.empty() && currentPathIndex < currentPath.size()) {
        sf::Vector2f nextWaypoint = currentPath[currentPathIndex];
        bool hitObstacle = moveToward(nextWaypoint, dt, blockedPolygons); // returns true if collision detected

        if (hitObstacle) {
            // Enter backing up state
            state = PoliceState::BackingUp;
            backedUpSoFar = 0.f;

            sf::Vector2f currentPos = getPosition();
            sf::Vector2f dir = nextWaypoint - currentPos;
            float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
            if (len > 0.01f)
                backUpDirection = -dir / len;

            return; // stop further movement this frame
        }

        float distanceToWaypoint = std::hypot(nextWaypoint.x - getPosition().x, nextWaypoint.y - getPosition().y);
        if (distanceToWaypoint < PATHFINDING_GRID_SIZE / 2.0f) {
            currentPathIndex++;
            if (currentPathIndex >= currentPath.size()) {
                currentPath.clear();
                currentPathIndex = 0;
                if (state == PoliceState::Idle) {
                    setRandomWanderDestination(MAP_BOUNDS);
                }
            }
        }
    }

    // Animation update
    animationTimer += dt;
    if (animationTimer >= animationSpeed) {
        animationTimer = 0.f;
        bool isMoving = !currentPath.empty() && currentPathIndex < currentPath.size();
        if (isMoving) {
            currentFrame = (currentFrame + 1) % framesPerRow;
        }
        else {
            currentFrame = 0;
        }
    }

    sprite.setTextureRect({
        currentFrame * frameWidth,
        0,
        frameWidth,
        frameHeight
        });
}


bool Police::moveToward(const sf::Vector2f& target, float dt, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) {
    sf::Vector2f currentPos = getPosition();
    sf::Vector2f dir = target - currentPos;
    float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    if (len < 0.01f) return false; // No movement needed

    dir /= len;

    float moveDistance = speed * dt;
    float stepSize = 5.f; // small step for collision checks
    float distanceToMove = std::min(moveDistance, len);

    for (float traveled = 0.f; traveled < distanceToMove; traveled += stepSize) {
        float segment = std::min(stepSize, distanceToMove - traveled);
        sf::Vector2f nextPos = currentPos + dir * (traveled + segment);

        if (checkCollision(currentPos, nextPos, blockedPolygons, getCollisionRadius())) {
            std::cout << "Police: Collision detected, stopping movement at segment " << traveled << std::endl;
            return true; // collision detected
        }

        currentPos = nextPos;
    }

    // No collision, update position and rotation
    float angle = std::atan2(dir.y, dir.x) * 180.f / M_PI;
    sprite.setRotation(angle - 270.f);
    sprite.setPosition(currentPos);

    return false; // no collision
}



void Police::draw(sf::RenderWindow& window) {
    window.draw(sprite);
}

void Police::takeDamage(int amount) {
    health -= amount;
    if (health < 0) health = 0;
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
    }
    else {
        // Fallback: if many tries fail, pick a point towards center or a default safe spot
        wanderDestination = sf::Vector2f(mapBounds.width / 2.f, mapBounds.height / 2.f);
    }
    //std::cout << "Police (Idle): New wander destination set to (" << wanderDestination.x << ", " << wanderDestination.y << ")" << std::endl;
    currentPath.clear();      // Clear old path
    currentPathIndex = 0;     // Reset index
    repathTimer = 0.f;        // Allow immediate repathing for new wander destination
    pathFailCooldown = 0.f;   // Reset path fail cooldown
}

bool Police::checkCollision(const sf::Vector2f& currentPos, const sf::Vector2f& nextPos, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons, float radius) {
    // בדוק קטע בין currentPos ל-nextPos או נקודות ביניים
    const int steps = 5;
    for (int i = 1; i <= steps; ++i) {
        float t = i / float(steps);
        sf::Vector2f interpPos = currentPos + (nextPos - currentPos) * t;

        // אפשר לבדוק אם circle סביב interpPos חופף פוליגון חסום
        for (const auto& poly : blockedPolygons) {
            if (CollisionUtils::pointInPolygon(interpPos, poly)) {
                return true; // התנגשות
            }
        }
    }
    return false;
}
