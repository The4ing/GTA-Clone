#include "PoliceTank.h"
#include "GameManager.h"
#include "Player.h"
#include "ResourceManager.h" // For textures
#include "PoliceManager.h"   // For pathfinding throttle
#include "CollisionUtils.h"  // For basic collision checks
#include <cmath>
#include <iostream>
#include "Pedestrian.h"
#include "Vehicle.h"
#include <algorithm>    

PoliceTank::PoliceTank(GameManager& gameManager, const sf::Vector2f& startPosition)
    : Vehicle(),
    m_gameManager(gameManager),
    m_pathfinder(*gameManager.getPathfindingGrid()),
    m_currentPathIndex(0),
    m_repathTimer(0.f),
    // m_targetPosition(startPosition) { // Replaced by m_currentTargetPosition
    m_currentTargetPosition(startPosition),
    m_tankState(TankState::Chasing) { // Initialize tank state

    std::cout << "tank has spawned";

    // --- Body ---
    Vehicle::setTexture(ResourceManager::getInstance().getTexture("policeTank"));
    Vehicle::getSprite().setOrigin(
        Vehicle::getSprite().getTextureRect().width / 2.f,
        Vehicle::getSprite().getTextureRect().height / 2.f
    );
    Vehicle::getSprite().setScale(0.2f, 0.2f);
    Vehicle::setPosition(startPosition);

    // --- Turret ---
    m_turretSprite.setTexture(ResourceManager::getInstance().getTexture("tankBody"));
    if (m_turretSprite.getTexture()) {
        m_turretSprite.setOrigin(
            m_turretSprite.getTexture()->getSize().x / 2.f,
            m_turretSprite.getTexture()->getSize().y / 2.f
        );
    }
    else {
        std::cerr << "PoliceTank turret texture not loaded!" << std::endl;
    }

    m_turretSprite.setScale(Vehicle::getSprite().getScale());
    m_turretSprite.setPosition(startPosition);

    m_health = 500;
    m_baseSpeed = 10.f; // Use m_baseSpeed, m_currentSpeed initialized in header
    m_currentSpeed = m_baseSpeed; // Initialize current speed
}

bool PoliceTank::hasClearLineOfSight(const sf::Vector2f& targetPos, const QuadTree<std::vector<sf::Vector2f>>& blockedPolyTree) const {
    sf::Vector2f startPos = getPosition();
    sf::Vector2f direction = targetPos - startPos;
    float distance = std::hypot(direction.x, direction.y);

    if (distance == 0.f) {
        return true; // Target is at the same position, clear LOS.
    }

    direction /= distance; // Normalize

    const float stepSize = 10.0f; // Check every 10 pixels, adjust as needed for performance/accuracy
    int numSteps = static_cast<int>(distance / stepSize);

    for (int i = 1; i < numSteps; ++i) { // Start from step 1 to avoid checking inside self, end before target
        sf::Vector2f currentPoint = startPos + direction * (i * stepSize);
        if (CollisionUtils::isInsideBlockedPolygon(currentPoint, blockedPolyTree)) {
            //std::cout << "LOS blocked at: " << currentPoint.x << "," << currentPoint.y << std::endl;
            return false; // Obstructed
        }
    }
    // Optionally, also check the target point itself if needed, though usually not for LOS.
    // if (CollisionUtils::isInsideBlockedPolygon(targetPos, blockedPolyTree)) return false; 
    return true; // Clear path
}


void PoliceTank::update(float dt, Player& player, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) {
    Vehicle::update(dt, blockedPolygons); // Base vehicle update (if any)

    updateAIBehavior(dt, player, blockedPolygons);
    //aimAndFire(player, dt);


    if (m_cannonCooldownTimer > 0.f) {
        m_cannonCooldownTimer -= dt;
    }

    const sf::Vector2f turretOffset(34.f, -23.f); // Offset from tank centre to turret origin

    // Rotate the offset by the tank body's current rotation so the turret stays aligned
    float bodyRotationRad = Vehicle::getSprite().getRotation() * 3.14159f / 180.f;
    sf::Vector2f rotatedOffset(
        turretOffset.x * std::cos(bodyRotationRad) - turretOffset.y * std::sin(bodyRotationRad),
        turretOffset.x * std::sin(bodyRotationRad) + turretOffset.y * std::cos(bodyRotationRad)
    );
    m_turretSprite.setPosition(getPosition() + rotatedOffset);
    std::cout << "Tank Position: ("
        << getPosition().x << ", "
        << getPosition().y << ")" << std::endl;
    // Aim and potentially fire after updating the turret's position so rotation smoothing uses the correct origin
    aimAndFire(player, dt);

    // Despawn check (moved from the end of updateAIBehavior to general update)
    if (m_tankState == TankState::Retreating && !m_readyForCleanup) { // only check if retreating and not already marked
        const sf::View& gameView = m_gameManager.getGameView();
        sf::FloatRect viewRect(gameView.getCenter() - gameView.getSize() / 2.f, gameView.getSize());
        // Add a buffer, so it's truly off-screen
        float buffer = 100.f; // Tank's approximate size or desired off-screen margin
        viewRect.left -= buffer;
        viewRect.top -= buffer;
        viewRect.width += 2 * buffer;
        viewRect.height += 2 * buffer;

        if (!viewRect.contains(getPosition())) {
            m_readyForCleanup = true;
            // std::cout << "Tank is off-screen and ready for cleanup." << std::endl;
        }
    }
}

void PoliceTank::updateAIBehavior(float dt, Player& player, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) {
    m_repathTimer += dt;
    int wantedLevel = player.getWantedLevel();

    m_distanceToPlayer = std::hypot(player.getPosition().x - getPosition().x, player.getPosition().y - getPosition().y);

    if (m_readyForCleanup) {
        m_currentPath.clear();
        return;
    }

    // Determine current state: Chasing or Retreating
    if (wantedLevel < 5 && m_tankState == TankState::Chasing) {
        m_tankState = TankState::Retreating;
        m_hasLineOfSightToPlayer = false; // No longer interested in shooting player

        sf::Vector2f playerPos = player.getPosition();
        sf::Vector2f currentPos = getPosition();
        sf::Vector2f directionAwayFromPlayer = currentPos - playerPos;
        float distToPlayerVec = std::hypot(directionAwayFromPlayer.x, directionAwayFromPlayer.y);

        if (distToPlayerVec < 10.f) { // If too close or on top, pick a random direction
            float randomAngle = (static_cast<float>(rand()) / RAND_MAX) * 2.f * M_PI;
            directionAwayFromPlayer = sf::Vector2f(std::cos(randomAngle), std::sin(randomAngle));
        }
        else if (distToPlayerVec > 0.001f) { // Normalize if not zero vector
            directionAwayFromPlayer /= distToPlayerVec;
        }
        else { // Default to a random direction if exactly on player
            float randomAngle = (static_cast<float>(rand()) / RAND_MAX) * 2.f * M_PI;
            directionAwayFromPlayer = sf::Vector2f(std::cos(randomAngle), std::sin(randomAngle));
        }

        float retreatDistance = 2000.f; // Define a far-off target point
        m_currentTargetPosition = currentPos + directionAwayFromPlayer * retreatDistance;

        m_currentPath.clear();
        m_currentPathIndex = 0;
        m_repathTimer = REPATH_COOLDOWN; // Force path attempt soon
    }
    else if (wantedLevel >= 5 && m_tankState == TankState::Retreating) {
        // If wanted level goes back up, switch back to chasing (unless already despawning)
        if (!m_readyForCleanup) {
            m_tankState = TankState::Chasing;
            m_currentPath.clear();
            m_currentPathIndex = 0;
            m_repathTimer = REPATH_COOLDOWN; // Force path attempt soon
        }
    }


    if (m_tankState == TankState::Retreating) {
        // Pathfinding for retreating
        bool needsNewPath = m_currentPath.empty() || m_currentPathIndex >= m_currentPath.size();
        if (needsNewPath && m_repathTimer >= REPATH_COOLDOWN && PoliceManager::canRequestPath()) {
            PoliceManager::recordPathfindingCall();
            m_currentPath = m_pathfinder.findPath(getPosition(), m_currentTargetPosition);
            m_currentPathIndex = 0;
            m_repathTimer = 0.f;
            if (m_currentPath.empty()) {
                // std::cout << "PoliceTank failed to find retreat path. Will become ready for cleanup if off-screen." << std::endl;
                // It might get stuck, but the off-screen check in update() will eventually clean it up.
            }
        }
        else if (needsNewPath && m_repathTimer >= REPATH_COOLDOWN) {
            m_repathTimer = 0.f; // Throttled, reset to try again
        }
    }
    else { // TankState::Chasing
        m_currentTargetPosition = player.getPosition();
        m_hasLineOfSightToPlayer = hasClearLineOfSight(m_currentTargetPosition, m_gameManager.getBlockedPolyTree());

        if (m_hasLineOfSightToPlayer && m_distanceToPlayer <= STOP_DISTANCE) {
            m_currentPath.clear(); // Stop movement if player is seen and close enough
            m_currentPathIndex = 0;
        }
        else {
            // Pathfinding to player
            bool needsNewPath = m_currentPath.empty() || m_currentPathIndex >= m_currentPath.size();
            if (!needsNewPath) { // Check if player moved significantly from path target
                float dx = m_currentTargetPosition.x - m_currentPath.back().x;
                float dy = m_currentTargetPosition.y - m_currentPath.back().y;
                if ((dx * dx + dy * dy) > PLAYER_MOVE_THRESHOLD_FOR_REPATH_SQ) {
                    needsNewPath = true;
                }
            }

            if (needsNewPath && m_repathTimer >= REPATH_COOLDOWN && PoliceManager::canRequestPath()) {
                PoliceManager::recordPathfindingCall();
                m_currentPath = m_pathfinder.findPath(getPosition(), m_currentTargetPosition);
                m_currentPathIndex = 0;
                m_repathTimer = 0.f;
                if (m_currentPath.empty()) {
                    // std::cout << "PoliceTank failed to find path to player." << std::endl;
                }
            }
            else if (needsNewPath && m_repathTimer >= REPATH_COOLDOWN) {
                m_repathTimer = 0.f; // Throttled
            }
        }
    }

    updateTankMovementAsCar(dt, player, blockedPolygons); // Call new movement function
}

// OLD updateMovement function is now removed.

void PoliceTank::updateTankMovementAsCar(float dt, Player& player, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) {
    // If chasing and has LOS and is close enough, don't move.
    if (m_tankState == TankState::Chasing && m_hasLineOfSightToPlayer && m_distanceToPlayer <= STOP_DISTANCE) {
        m_currentSpeed = 0;
        return;
    }

    m_currentSpeed = m_baseSpeed;

    if (!m_currentPath.empty() && m_currentPathIndex < m_currentPath.size()) {
        sf::Vector2f nextWaypoint = m_currentPath[m_currentPathIndex];
        sf::Vector2f directionToWaypoint = nextWaypoint - getPosition();
        float distanceToWaypoint = std::hypot(directionToWaypoint.x, directionToWaypoint.y);

        if (distanceToWaypoint > 0.01f) {
            sf::Vector2f normalizedDirection = directionToWaypoint / distanceToWaypoint;
            float moveStep = m_currentSpeed * dt;

            sf::Vector2f nextPosCandidate;
            bool advancingToNext = false;

            if (distanceToWaypoint <= moveStep) {
                // Snap to waypoint directly and mark for next step
                nextPosCandidate = nextWaypoint;
                advancingToNext = true;
            }
            else {
                nextPosCandidate = getPosition() + normalizedDirection * moveStep;
            }

            bool collision = CollisionUtils::isInsideBlockedPolygon(nextPosCandidate, m_gameManager.getBlockedPolyTree());
            if (!collision) {
                Vehicle::setPosition(nextPosCandidate);

                float targetAngle = std::atan2(normalizedDirection.y, normalizedDirection.x) * 180.f / M_PI + 90.f;
                float currentAngle = Vehicle::getSprite().getRotation();

                // ???? ????? ???? ??? -180 ?-180
                float angleDiff = targetAngle - currentAngle;
                while (angleDiff > 180.f) angleDiff -= 360.f;
                while (angleDiff < -180.f) angleDiff += 360.f;

                // ????? — ??? ????? ????? ?? ????? (????? ???????)
                float maxRotationPerFrame = 90.f * dt; // ???? ????? ??60 ?? 120 ??? ??? ?? ??? ????

                if (std::abs(angleDiff) < maxRotationPerFrame) {
                    Vehicle::getSprite().setRotation(targetAngle); // ???? ????? — ????? ??????
                }
                else {
                    Vehicle::getSprite().rotate((angleDiff > 0 ? 1.f : -1.f) * maxRotationPerFrame);
                }


                if (advancingToNext || distanceToWaypoint < TARGET_REACHED_DISTANCE) {
                    m_currentPathIndex++;
                    if (m_currentPathIndex >= m_currentPath.size()) {
                        m_currentPath.clear();
                        if (m_tankState == TankState::Retreating) {
                            m_readyForCleanup = true;
                        }
                    }
                }
            }
            else {
                // Collision encountered, clear path
                m_currentPath.clear();
                m_currentPathIndex = 0;
            }
        }
    }
    else if (m_currentPath.empty() && m_tankState == TankState::Retreating) {
        m_readyForCleanup = true;
    }
}



void PoliceTank::aimAndFire(Player& player, float dt) {
    sf::Vector2f playerPos = player.getPosition();
    sf::Vector2f directionToPlayer = playerPos - getPosition();
    float distanceToPlayer = std::hypot(directionToPlayer.x, directionToPlayer.y);

    // Turret rotation
    float targetTurretAngle = std::atan2(directionToPlayer.y, directionToPlayer.x) * 180.f / M_PI + 90.f;
    float currentTurretAngle = m_turretSprite.getRotation();
    float turretAngleDiff = targetTurretAngle - currentTurretAngle;
    while (turretAngleDiff > 180.f) turretAngleDiff -= 360.f;
    while (turretAngleDiff < -180.f) turretAngleDiff += 360.f;

    float turretRotationThisFrame = m_turretRotationSpeed * dt;
    if (std::abs(turretAngleDiff) < turretRotationThisFrame) {
       // m_turretSprite.setRotation(targetTurretAngle);
    }
    else {
        m_turretSprite.rotate(turretAngleDiff > 0 ? turretRotationThisFrame : -turretRotationThisFrame);
    }

    // Firing logic: requires LOS, in range, turret aligned, and cooldown ready
    if (m_hasLineOfSightToPlayer && distanceToPlayer <= CANNON_RANGE && std::abs(turretAngleDiff) < 10.f && m_cannonCooldownTimer <= 0.f) {
        // std::cout << "Tank firing cannon! LOS confirmed." << std::endl;

        // Calculate projectile spawn position based on turret's current rotation and position
        // Assuming cannonTipOffset is from the turret's origin/center
        float turretAngleRad = (m_turretSprite.getRotation() - 90.f) * M_PI / 180.f; // Convert current turret sprite angle to world radians
        // cannonTipOffset.x is along the barrel, Y is perpendicular (usually 0 for simple barrel)
        sf::Vector2f cannonTipLocalOffset(50.f, 0.f); // Offset from turret's origin to its tip 

        sf::Vector2f rotatedTip = sf::Vector2f(
            cannonTipLocalOffset.x * std::cos(turretAngleRad) - cannonTipLocalOffset.y * std::sin(turretAngleRad),
            cannonTipLocalOffset.x * std::sin(turretAngleRad) + cannonTipLocalOffset.y * std::cos(turretAngleRad)
        );
        // Projectile spawns from the turret's current visual position plus the rotated offset
        sf::Vector2f projectileSpawnPos = m_turretSprite.getPosition() + rotatedTip;

        // Fire direction should be from the actual spawn point towards the player target
        sf::Vector2f fireDirection = playerPos - projectileSpawnPos;
        float fireDirectionMagnitude = std::hypot(fireDirection.x, fireDirection.y);

        if (fireDirectionMagnitude > 0.001f) { // Normalize, guard against zero vector if player is exactly at spawn
            fireDirection /= fireDirectionMagnitude;
        }
        else {
            // Fallback: fire along turret's current facing if player is exactly at projectileSpawnPos (very unlikely)
            // This uses the already calculated turretAngleRad which is the direction the turret is pointing.
            fireDirection = sf::Vector2f(std::cos(turretAngleRad), std::sin(turretAngleRad));
        }

        m_gameManager.addBullet(projectileSpawnPos, fireDirection, BulletType::TankShell); // Use TankShell type
        m_cannonCooldownTimer = CANNON_FIRE_RATE;
    }
}

void PoliceTank::draw(sf::RenderTarget& target) {
    Vehicle::draw(target); // Draw tank body (assuming Vehicle::draw handles its sprite)
    target.draw(m_turretSprite); // Draw turret on top
}

sf::Vector2f PoliceTank::getPosition() const {
    return Vehicle::getPosition(); // Delegate to base class
}

void PoliceTank::setPosition(const sf::Vector2f& pos) {
    Vehicle::setPosition(pos);    // Delegate to base class
}

bool PoliceTank::isDestroyed() const {
    return m_health <= 0;
}

void PoliceTank::takeDamage(int amount) {
    m_health -= amount;
    if (m_health < 0) m_health = 0;
    // std::cout << "Tank took " << amount << " damage, health: " << m_health << std::endl;
    if (isDestroyed()) {
        // std::cout << "Tank destroyed!" << std::endl;
        // m_gameManager.createExplosion(getPosition(), ExplosionSize::VERY_LARGE); // Example
    }
}

bool PoliceTank::attemptRunOverPedestrian(Pedestrian& ped) {
    float distSq = std::pow(getPosition().x - ped.getPosition().x, 2.f) +
        std::pow(getPosition().y - ped.getPosition().y, 2.f);
    float combined = (getSprite().getGlobalBounds().width / 2.f) + ped.getCollisionRadius();
    if (distSq < combined * combined) {
        ped.takeDamage(9999.f);
        return true;
    }
    return false;
}

bool PoliceTank::attemptRunOverVehicle(Vehicle& vehicle) {
    if (&vehicle == this) return false;
    if (getSprite().getGlobalBounds().intersects(vehicle.getSprite().getGlobalBounds())) {
        vehicle.setDestroyed(true);
        return true;
    }
    return false;
}

bool PoliceTank::canSeePlayer(const Player& player, const std::vector<std::vector<sf::Vector2f>>& obstacles) {
    sf::Vector2f selfPos = getPosition();
    sf::Vector2f playerPos = player.getPosition();
    sf::Vector2f directionToPlayer = playerPos - selfPos;
    float distanceToPlayer = std::hypot(directionToPlayer.x, directionToPlayer.y);

    if (distanceToPlayer > m_visionDistance) {
        return false;
    }

    if (distanceToPlayer > 0.001f) {
        float unitAngleRad = (getSprite().getRotation() - 90.f) * static_cast<float>(M_PI) / 180.f;
        sf::Vector2f forwardVector(std::cos(unitAngleRad), std::sin(unitAngleRad));
        sf::Vector2f normalizedDirToPlayer = directionToPlayer / distanceToPlayer;
        float dotProduct = forwardVector.x * normalizedDirToPlayer.x + forwardVector.y * normalizedDirToPlayer.y;
        dotProduct = std::clamp(dotProduct, -1.f, 1.f);
        float angleBetweenActualRad = std::acos(dotProduct);
        float fovThresholdRad = (m_fieldOfViewAngle / 2.f) * static_cast<float>(M_PI) / 180.f;
        if (angleBetweenActualRad > fovThresholdRad) {
            return false;
        }
    }

    const int LOS_SAMPLE_POINTS = 3;
    for (int i = 1; i <= LOS_SAMPLE_POINTS; ++i) {
        sf::Vector2f testPoint = selfPos + directionToPlayer * (static_cast<float>(i) / LOS_SAMPLE_POINTS);
        if (CollisionUtils::isInsideBlockedPolygon(testPoint, obstacles)) {
            return false;
        }
    }
    return true;
}
