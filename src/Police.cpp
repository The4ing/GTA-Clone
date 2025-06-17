#include "Police.h"
#include "ResourceManager.h"
#include <cmath>
#include <algorithm>
#include <iostream>
#include "CollisionUtils.h"

const sf::FloatRect MAP_BOUNDS(0.f, 0.f, 4640.f, 4670.f);

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
}

//void Police::update(float dt, const QuadTree<std::vector<sf::Vector2f>>& blockedPolyTree) {
//    static float pathFailCooldown = 0.f;
//    repathTimer += dt;
//    float distToTarget = std::hypot(targetPos.x - getPosition().x, targetPos.y - getPosition().y);
//
//    if (pathFailCooldown > 0.f) {
//        pathFailCooldown -= dt;
//        return;
//    }
//
//    // --- מעבר סטייטים ---
//    if (distToTarget <= detectionRadius) {
//        if (state != PoliceState::Chasing) {
//            state = PoliceState::Chasing;
//            currentFrame = 0;
//            repathTimer = 2.0f;
//        }
//    }
//    else if (state == PoliceState::Chasing && distToTarget > detectionRadius + 30.f) {
//        state = PoliceState::Idle;
//        currentFrame = 0;
//        // אל תאפס כאן את המסלול! תן לקוד לגלות לבד אם הגיע לסוף
//        // currentPath.clear();
//        // currentPathIndex = 0;
//    }
//
//    // --- ניהול מסלול (רק כשהסתיים או צריך יעד חדש) ---
//    if (state == PoliceState::Chasing) {
//        if (repathTimer > 1.0f || currentPath.empty() || currentPathIndex >= currentPath.size()) {
//            //currentPath = findPathWorld(
//            //    getPosition(), targetPos,
//            //    24.0f,
//            //    blockedPolyTree,
//            //    getCollisionRadius(),
//            //    8000
//            //);
//            //std::cout << "[PATHFIND][Chasing] Path size: " << currentPath.size() << std::endl;
//            currentPathIndex = 0;
//            repathTimer = 0.f;
//            if (currentPath.empty()) {
//                // std::cout << "Police: Could not find path to player! Waiting 1.5 seconds...\n";
//                pathFailCooldown = 3.f;
//                return;
//            }
//        }
//    }
//    else if (state == PoliceState::Idle) {
//
//        float idleRepathTimer = 0.f;
//        idleRepathTimer += dt;
//        const float minDistanceForNewGoal = 1500.f;
//
//        bool needNewGoal = currentPath.empty() || currentPathIndex >= currentPath.size() || idleRepathTimer > 3.f;
//        if (needNewGoal) {
//            const int MAX_ATTEMPTS = 3;
//            bool found = false;
//            for (int attempt = 0; attempt < MAX_ATTEMPTS && !found; ++attempt) {
//                float x = static_cast<float>(rand() % int(MAP_BOUNDS.width));
//                float y = static_cast<float>(rand() % int(MAP_BOUNDS.height));
//                sf::Vector2f candidate(x, y);
//                float dist = std::hypot(candidate.x - getPosition().x, candidate.y - getPosition().y);
//                //  std::cout << "[Idle] Attempt " << attempt << ": (" << x << "," << y << "), dist=" << dist;
//
//                if (dist > minDistanceForNewGoal) {
//                    //    std::cout << " [DIST OK]";
//                    //if (!isBlocked(candidate, blockedPolyTree, getCollisionRadius())) {
//                    //    std::cout << " [FREE!]" << std::endl;
//                    //    wanderDestination = candidate;
//                    //    found = true;
//                    //    break;
//                    //}
//                    //else {
//                    //    std::cout << " [BLOCKED]" << std::endl;
//                    //}
//                }
//                else {
//                    std::cout << " [TOO CLOSE]" << std::endl;
//                }
//            }
//
//            if (found) {
//                //currentPath = findPathWorld(
//                //    getPosition(), wanderDestination,
//                //    24.0f,
//                //    blockedPolyTree,
//                //    getCollisionRadius(),
//                //    1000
//                //);
//               // std::cout << "[PATHFIND][Idle] Path size: " << currentPath.size() << std::endl;
//                if (!currentPath.empty()) {
//                    std::cout << "[PATH] ";
//                    for (auto& pt : currentPath) std::cout << "(" << pt.x << "," << pt.y << ") ";
//                    std::cout << std::endl;
//                }
//                currentPathIndex = 0;
//                idleRepathTimer = 0.f;
//                if (currentPath.empty()) {
//                    pathFailCooldown = 5.f;
//                    return;
//                }
//            }
//            else {
//                pathFailCooldown = 5.f;
//                return;
//            }
//        }
//    }
//
//    // --- תנועה לאורך המסלול (רציפה!) ---
//    if (!currentPath.empty() && currentPathIndex < currentPath.size()) {
//        sf::Vector2f target = currentPath[currentPathIndex];
//        sf::Vector2f pos = getPosition();
//        sf::Vector2f toTarget = target - pos;
//        float dist = std::sqrt(toTarget.x * toTarget.x + toTarget.y * toTarget.y);
//
//        float moveStep = speed * dt;
//        if (dist <= moveStep) {
//            // הגיע ליעד: מקדם לנקודה הבאה
//            sprite.setPosition(target);
//            ++currentPathIndex;
//            if (currentPathIndex >= currentPath.size()) {
//                currentPath.clear();
//                currentPathIndex = 0;
//            }
//        }
//        else {
//            // זזים חלק בכיוון היעד
//            sf::Vector2f dir = toTarget / dist;
//            sf::Vector2f nextPos = pos + dir * moveStep;
//
//            // בדיקת התנגשות
//            bool collision = false;
//            float radius = getCollisionRadius();
//            sf::FloatRect area(nextPos.x - radius, nextPos.y - radius, radius * 2, radius * 2);
//            auto nearby = blockedPolyTree.query(area);
//            for (const auto& poly : nearby) {
//                if (CollisionUtils::pointInPolygon(nextPos, poly)) {
//                    collision = true;
//                    break;
//                }
//            }
//
//            if (!collision) {
//                sprite.move(dir * moveStep);
//            }
//        }
//    }
//
//    // --- אנימציה ---
//    animationTimer += dt;
//    if (animationTimer >= animationSpeed) {
//        animationTimer = 0.f;
//        if (std::abs(sprite.getRotation()) > 0.1f && distToTarget > 1.f) {
//            currentFrame = (currentFrame + 1) % framesPerRow;
//        }
//        else {
//            currentFrame = 0;
//        }
//    }
//    sprite.setTextureRect({
//        currentFrame * frameWidth,
//        0,
//        frameWidth,
//        frameHeight
//        });
//}

void Police::update(float dt, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) {
    float dist = std::hypot(targetPos.x - getPosition().x, targetPos.y - getPosition().y);

    debugPrintTimer += dt;
    if (debugPrintTimer >= 2.f) { // הדפסה כל 2 שניות
        float dist = std::hypot(wanderDestination.x - getPosition().x, wanderDestination.y - getPosition().y);
        debugPrintTimer = 0.f;
    }


    if (dist <= detectionRadius) {
        if (state != PoliceState::Chasing) {
            state = PoliceState::Chasing;
            currentFrame = 0;
        }
    }
    else if (state == PoliceState::Chasing && dist > detectionRadius + 30.f) {
        state = PoliceState::Idle;
        currentFrame = 0;
    }

    if (state == PoliceState::Chasing) {
        moveToward(targetPos, dt, blockedPolygons);
    }
    else if (state == PoliceState::Idle) {
        float distToDest = std::hypot(wanderDestination.x - getPosition().x, wanderDestination.y - getPosition().y);
        if (distToDest < 10.f) { // Arrived
            setRandomWanderDestination(MAP_BOUNDS);
        }
        moveToward(wanderDestination, dt, blockedPolygons);
    }

    //else if (state == PoliceState::Idle) {
    //    wanderTimer -= dt;
    //    if (wanderTimer <= 0.f) {
    //        targetPos = getPosition() + sf::Vector2f(rand() % 100 - 50, rand() % 100 - 50);
    //        wanderTimer = 2.f + static_cast<float>(rand() % 300) / 100.f; 
    //    }
    //    moveToward(targetPos, dt, blockedPolygons);
    //}

    animationTimer += dt;
    if (animationTimer >= animationSpeed) {
        animationTimer = 0.f;
        if (std::abs(sprite.getRotation()) > 0.1f && (std::abs(targetPos.x - getPosition().x) > 1.f || std::abs(targetPos.y - getPosition().y) > 1.f)) {
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

void Police::moveToward(const sf::Vector2f& target, float dt, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) {
    sf::Vector2f dir = target - getPosition();
    float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    if (len > 0.f)
        dir /= len;
    else
        dir = sf::Vector2f(0.f, 1.f);


    float angle = std::atan2(dir.y, dir.x) * 180.f / M_PI;
    sprite.setRotation(angle - 270.f);

    sf::Vector2f nextPos = sprite.getPosition() + dir * speed * dt;
    bool collision = false;
    for (const auto& poly : blockedPolygons) {
        if (CollisionUtils::pointInPolygon(nextPos, poly)) {
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
}

void Police::takeDamage(int amount) {
    health -= amount;
    if (health < 0) health = 0;
}

bool Police::isDead() const {
    return health <= 0;
}

float Police::getCollisionRadius() const {
    return 12.f;
}

void Police::setTargetPosition(const sf::Vector2f& pos) {
    targetPos = pos;
}

void Police::setRandomWanderDestination(const sf::FloatRect& mapBounds) {
    sf::Vector2f candidate;
    int tries = 0;
    bool found = false;

    do {
        float angle = static_cast<float>(rand()) / RAND_MAX * 2 * M_PI;
        float radius = static_cast<float>(rand()) / RAND_MAX * 500.f;
        candidate = getPosition() + sf::Vector2f(std::cos(angle), std::sin(angle)) * radius;
        if (mapBounds.contains(candidate))
            found = true;
        ++tries;
    } while (!found && tries < 20);

    wanderDestination = candidate;
}
