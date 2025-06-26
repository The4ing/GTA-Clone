#include "Police.h"
#include "ResourceManager.h"
#include <cmath>
#include <algorithm>
#include <iostream>
#include "CollisionUtils.h"


Police::Police(sf::Vector2f target) : targetPos(target), state(PoliceState::Idle), 
currentPathIndex(0), repathTimer(0.f), pathFailCooldown(0.f),
animationTimer(0.f), animationSpeed(0.005f), currentFrame(0),
framesPerRow(10) // 10 columns
{
    sprite.setTexture(ResourceManager::getInstance().getTexture("police"));
    sheetCols = 10;
    sheetRows = 10;
    frameWidth = sprite.getTexture()->getSize().x / sheetCols;
    frameHeight = sprite.getTexture()->getSize().y / sheetRows;
    sprite.setTextureRect({ 0, 0, frameWidth, frameHeight });
    sprite.setOrigin(frameWidth / 2.f, frameHeight / 2.f);
    
    sprite.setPosition(100, 100);
    sprite.setScale(0.15f, 0.15f);
    speed = 40.f;
    
    nextPauseTime = 30.f + static_cast<float>(rand()) / RAND_MAX * 30.f;  // בין 30 ל-60 שניות


    animationManager = std::make_unique<AnimationManager>(sprite, frameWidth, frameHeight, sheetCols, sheetRows); 
    initAnimations();

    setRandomWanderDestination(MAP_BOUNDS);

}

void Police::update(float dt, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) {
    repathTimer += dt;
    if (pathFailCooldown > 0.f) {
        pathFailCooldown -= dt;
    }

    if (isPaused) {
        
            pauseTimer -= dt;
        if (pauseTimer <= 0.f) {
            isPaused = false;
            // קבע מתי העצירה הבאה תתרחש מחדש
            nextPauseTime = 30.f + static_cast<float>(rand()) / RAND_MAX * 30.f;
        }
    }
    else {
        nextPauseTime -= dt;
        if (nextPauseTime <= 0.f) {
            // מתחיל עצירה
            isPaused = true;
            pauseTimer = 3;  // כאן תקבע את משך העצירה בשניות (לדוגמה, 3 שניות)
        }
    }

    // אם עצור - אל תבצע תזוזה או שינוי מצב
    if (isPaused) {
        // יכול לשים כאן גם אנימציית Idle או דומם
     //   animationManager->setAnimation("Idle_NoWeapon", true);
        setSpecificFrame(0, 0);
        // animationManager->update(dt);
        return; // מפסיק כאן ומונע תזוזה עד שהעצירה תסתיים
    }
    
    float distToPlayer = std::hypot(targetPos.x - getPosition().x, targetPos.y - getPosition().y);

    // State transition logic
    if (distToPlayer <= detectionRadius) {
        if (state != PoliceState::Chasing) {
            state = PoliceState::Chasing;
            currentPath.clear();
            currentPathIndex = 0;
            repathTimer = 1.0f;
        }
    }
    else if (state == PoliceState::Chasing && distToPlayer > detectionRadius + 30.f) {
        state = PoliceState::Idle;
        currentPath.clear();
        currentPathIndex = 0;
        setRandomWanderDestination(MAP_BOUNDS);
    }

   
    if (state == PoliceState::BackingUp) {
        float stepSize = speed * dt;
        if (backedUpSoFar < backUpDistance) {
            sf::Vector2f currentPos = getPosition();
            sf::Vector2f nextPos = currentPos + backUpDirection * stepSize;

            if (checkCollision(currentPos, nextPos, blockedPolygons, getCollisionRadius())) {
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
            return; 
        }
        else {
           
            backedUpSoFar = 0.f;
            state = PoliceState::Idle;
            currentPath.clear();
            currentPathIndex = 0;
            setRandomWanderDestination(MAP_BOUNDS);
            return;
        }
    }

   
    if (state == PoliceState::Chasing) {
        if (pathFailCooldown <= 0.f && (currentPath.empty() || currentPathIndex >= currentPath.size() || repathTimer > 1.0f)) {
            currentPath = pathfinder.findPath(getPosition(), targetPos, blockedPolygons, MAP_BOUNDS, PATHFINDING_GRID_SIZE);
            currentPathIndex = 0;
            repathTimer = 0.f;
            if (currentPath.empty()) {
                pathFailCooldown = 3.0f;
            }
        }
    }
    else if (state == PoliceState::Idle) {
        if (currentPath.empty() || currentPathIndex >= currentPath.size()) {
            if (pathFailCooldown <= 0.f) {
                currentPath = pathfinder.findPath(getPosition(), wanderDestination, blockedPolygons, MAP_BOUNDS, PATHFINDING_GRID_SIZE);
                currentPathIndex = 0;
                repathTimer = 0.f;
                if (currentPath.empty()) {
                    pathFailCooldown = 1.0f;
                    setRandomWanderDestination(MAP_BOUNDS);
                }
            }
        }
    }

    // Movement Execution along the path
    if (!currentPath.empty() && currentPathIndex < currentPath.size()) {
        sf::Vector2f nextWaypoint = currentPath[currentPathIndex];
        bool hitObstacle = moveToward(nextWaypoint, dt, blockedPolygons);

        if (hitObstacle) {
            state = PoliceState::BackingUp;
            backedUpSoFar = 0.f;

            sf::Vector2f dir = nextWaypoint - getPosition();
            float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
            if (len > 0.01f)
                backUpDirection = -dir / len;

            return; 
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

    animationTimer += dt;
    if (animationTimer >= animationSpeed) {
        animationTimer = 0.f;
        bool isMoving = !currentPath.empty() && currentPathIndex < currentPath.size();
        if (state == PoliceState::Idle) {
          animationManager->setAnimation(isMoving ? "Walk_NoWeapon" : "Idle_NoWeapon", true);
        }
        else if (state == PoliceState::Chasing) {
              animationManager->setAnimation(isMoving ? "Walk_Gun_1" : "Idle_Gun_1", true);
        }
    }
    else if (state == PoliceState::BackingUp) {

        animationManager->setAnimation("BackingUp", true);
    }
    animationManager->update(dt);
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
          //  std::cout << "Police: Collision detected, stopping movement at segment " << traveled << std::endl;
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



void Police::draw(sf::RenderTarget& window) {
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
void Police::initAnimations() {
    animationManager->addAnimation("Walk_NoWeapon", { 0, 0, 1, 6 });       // שורה 0 עד 1, עמודה 0 עד 6
    animationManager->addAnimation("Walk_Gun_1", { 1, 7, 3, 3 });       // שורה 1 עד 3, עמודה 7 עד 3 (שים לב: צריך לשים לב לסדר, כנראה {1,7,3,3} משמעותו שורה 1 עד 3, עמודה 7 עד 3 - אם זה לא הגיוני אפשר לתקן)
    animationManager->addAnimation("Walk_Gun_2", { 3, 4, 4, 8 });       // שורה 3 עד 4, עמודה 4 עד 8
    animationManager->addAnimation("Walk_Shield", { 4, 9, 6, 9 });       // שורה 4 עד 6, עמודה 9
    animationManager->addAnimation("Throw_Grenade", { 7, 0, 7, 9 });       // שורה 7, עמודה 0 עד 9
    animationManager->addAnimation("Baton_Attack", { 8, 0, 8, 8 });       // שורה 8, עמודה 0 עד 8
    animationManager->addAnimation("Dying", { 8, 9, 9, 7 });       // שורה 8 עד 9, עמודה 9 עד 7 (גם כאן, אם סדר העמודות הפוך יש לתקן)
}

void Police::setSpecificFrame(int row, int col) {
    sf::IntRect rect(col * frameWidth, row * frameHeight, frameWidth, frameHeight);
    sprite.setTextureRect(rect);
}