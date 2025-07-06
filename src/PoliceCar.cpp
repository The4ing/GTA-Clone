#include "PoliceCar.h"
#include "ResourceManager.h"
#include "GameManager.h" // For PathfindingGrid, Player access
#include "Player.h"      // For Player object
#include "PoliceManager.h" // For canRequestPath, recordPathfindingCall
#include "CollisionUtils.h" // For collision checks if needed
#include <cmath>        // For std::hypot, std::atan2, M_PI
#include <iostream>     // For debugging
#include "PatrolZone.h"
#include "Pedestrian.h"

PoliceCar::PoliceCar(GameManager& gameManager, const sf::Vector2f& startPosition)
    : Vehicle(), // Call base Vehicle constructor
    m_gameManager(gameManager),
    m_pathfinder(*gameManager.getPathfindingGrid()), // Initialize Pathfinder
    m_currentPathIndex(0),
    m_repathTimer(0.f),
    m_currentTargetPosition(-1.f, -1.f),
    m_isAmbient(true), // Explicitly initialize, though defaults in .h
    m_playerCausedWantedIncrease(false), // Explicitly initialize
    m_assignedZone(nullptr) // Initialize assigned zoneAdd commentMore actions
{

    // Use Vehicle's setTexture or manage sprite locally if Vehicle doesn't have one.
    // Assuming Vehicle class has a sprite or a way to set texture.
    // For now, let's assume PoliceCar manages its own m_sprite as defined in its header.
    m_sprite.setTexture(ResourceManager::getInstance().getTexture("PoliceCar")); // Ensure "police_car" texture key is correct
    if (m_sprite.getTexture()) {
        m_sprite.setOrigin(m_sprite.getTexture()->getSize().x / 2.f, m_sprite.getTexture()->getSize().y / 2.f);
    }
    else {
        std::cerr << "PoliceCar texture not loaded!" << std::endl;
        m_sprite.setOrigin(0, 0); // Default origin
    }
    m_sprite.setScale(0.09f, 0.09f); // Adjust scale as needed
    setPosition(startPosition); // Set initial position using Vehicle's setPosition or m_sprite.setPosition
    // Vehicle class should handle its internal position state.
    // If PoliceCar uses its own m_sprite directly for position:
    m_sprite.setPosition(startPosition);


    // If Vehicle class has speed, set it there. Otherwise, use m_speed from PoliceCar.h
    // this->speed = m_speed; // Example if Vehicle has a public 'speed' member
    m_carState = CarState::AmbientDriving; // Default state
    m_currentSpeed = m_speed;
    m_bumpCount = 0;
    m_playerHitCount = 0; // Initialize player hit count
    m_requestOfficerExit = false;
}
bool PoliceCar::isRetreating() const {
    return m_carState == CarState::Retreating;
}

void PoliceCar::startRetreating(const sf::Vector2f& retreatTarget) {
    if (m_carState == CarState::Retreating) return;

    m_carState = CarState::Retreating;
    m_isAmbient = false; // No longer ambient if retreating
    m_currentTargetPosition = retreatTarget; // Store the final off-screen target
    m_currentPath.clear();
    m_currentPathIndex = 0;
    m_repathTimer = REPATH_COOLDOWN; // Force immediate path attempt

    if (PoliceManager::canRequestPath()) {
        PoliceManager::recordPathfindingCall();
        m_currentPath = m_pathfinder.findPath(getPosition(), retreatTarget);
        if (m_currentPath.empty()) {
            // std::cout << "PoliceCar " << this << ": Failed to find retreat path. Marking for cleanup." << std::endl;
            needsCleanup = true;
        }
        else {
            // std::cout << "PoliceCar " << this << ": Starting retreat to (" << retreatTarget.x << "," << retreatTarget.y << ")" << std::endl;
        }
    }
    else {
        // std::cout << "PoliceCar " << this << ": Pathfinding throttled for retreat. Marking for cleanup." << std::endl;
        needsCleanup = true;
    }
}

void PoliceCar::setIsAmbient(bool isAmbient) {
    m_isAmbient = isAmbient;
    if (m_isAmbient && m_carState != CarState::Retreating) { // Don't switch to ambient if retreating
        m_carState = CarState::AmbientDriving;
    }
    else if (!m_isAmbient && m_carState == CarState::AmbientDriving) {
        m_carState = CarState::Chasing; // Default to chasing if not ambient and not retreating
    }
}

bool PoliceCar::isAmbient() const {
    return m_isAmbient;
}


void PoliceCar::update(float dt, Player& player, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) {
    // If car is static (e.g., officer exited) and not retreating, it should not update its movement.
    // The existing !hasDriver() check is for empty static cars (e.g. parked).
    // We also need to stop it if m_isStatic is true due to officer exiting, even if it technically still "has" an officer slot (that's now empty).
    if (m_isStatic && m_carState != CarState::Retreating) {
        // If it became static because officer exited, m_hasOfficerInside would be false.
        // If it was a pre-placed static car, hasDriver() (from Vehicle) would be false.
        // The key is m_isStatic being true and not in Retreating state.
        return;
    }

    Vehicle::update(dt, blockedPolygons);

    if (m_carState == CarState::Retreating) {
        m_repathTimer += dt; // Similar to chase, path to retreat target
        bool needsNewRetreatPath = m_currentPath.empty() || m_currentPathIndex >= m_currentPath.size();

        // No need to check if target moved significantly, as retreat target is fixed.
        // However, if path fails or is blocked, might need to repath or give up.

        if (needsNewRetreatPath && m_repathTimer >= REPATH_COOLDOWN && PoliceManager::canRequestPath()) {
            PoliceManager::recordPathfindingCall();
            m_currentPath = m_pathfinder.findPath(getPosition(), m_currentTargetPosition); // m_currentTargetPosition is the retreat spot
            m_currentPathIndex = 0;
            m_repathTimer = 0.f;
            if (m_currentPath.empty()) {
                // std::cout << "PoliceCar " << this << ": Failed to RE-find retreat path. Marking for cleanup." << std::endl;
                needsCleanup = true;
            }
        }
        else if (needsNewRetreatPath && m_repathTimer >= REPATH_COOLDOWN) {
            m_repathTimer = 0.f; // Throttled
            needsCleanup = true; // If throttled during retreat and no path, assume stuck
        }

        if (!m_currentPath.empty() && m_currentPathIndex < m_currentPath.size()) {
            sf::Vector2f nextWaypoint = m_currentPath[m_currentPathIndex];
            // Simplified movement logic (similar to chase, but towards m_currentTargetPosition)
            sf::Vector2f direction = nextWaypoint - getPosition();
            float distanceToWaypoint = std::hypot(direction.x, direction.y);
            if (distanceToWaypoint > 0.01f) direction /= distanceToWaypoint;

            sf::Vector2f currentPos = getPosition();
            float moveStep = m_currentSpeed * dt;
            sf::Vector2f nextPosCandidate = currentPos + direction * std::min(moveStep, distanceToWaypoint);

            // Basic collision check for retreating cars as well
            bool collision = false;
            // NOTE: This simple collision check might be insufficient. Ideally, use Vehicle's collision.
            for (const auto& poly : blockedPolygons) {
                if (CollisionUtils::pointInPolygon(nextPosCandidate, poly)) {
                    collision = true;
                    m_currentPath.clear();
                    m_currentPathIndex = 0;
                    // std::cout << "PoliceCar (Retreating): Collision, clearing path." << std::endl;
                    // Consider if it should try to repath or just give up (mark needsCleanup = true)
                    break;
                }
            }

            if (!collision) {
                setPosition(nextPosCandidate);
                m_sprite.setPosition(nextPosCandidate);
                if (distanceToWaypoint > 0.01f) {
                    float angle = std::atan2(direction.y, direction.x) * 180.f / M_PI;
                    m_sprite.setRotation(angle + 90.f);
                }
            }


            if (distanceToWaypoint < TARGET_REACHED_DISTANCE) {
                m_currentPathIndex++;
            }
            if (m_currentPathIndex >= m_currentPath.size()) {
                needsCleanup = true; // Reached end of retreat path
                // std::cout << "PoliceCar " << this << " reached retreat destination. Cleanup." << std::endl;
            }
        }
        else if (m_currentPath.empty() && m_carState == CarState::Retreating) {
            // No path to retreat, or path finished, mark for cleanup
            needsCleanup = true;
            // std::cout << "PoliceCar " << this << " has no/finished retreat path. Cleanup." << std::endl;
        }
        return; // Don't process other states if retreating
    }


    // --- Regular Update Logic (Ambient or Chasing) ---
    if (m_isAmbient && player.getWantedLevel() >= 3 && m_carState != CarState::Retreating) {
        setIsAmbient(false); // Transition to non-ambient (aggressive), which sets state to Chasing
    }

    if (m_carState == CarState::AmbientDriving) {
        // Ambient behavior logic (currently relies on Vehicle::update or simple driving)
    }
    else if (m_carState == CarState::Chasing) {
        // Non-ambient (aggressive) behavior:
        updateChaseBehavior(dt, player, blockedPolygons);

        if (m_bumpCooldown <= 0.f && attemptRunOverPlayer(player, blockedPolygons)) {
            player.takeDamage(static_cast<int>(m_currentSpeed / 4.f));
            m_bumpCooldown = 3.f;
        }
    }

    if (m_bumpCooldown > 0.f) {
        m_bumpCooldown -= dt;
    }
}

void PoliceCar::setPatrolZone(PatrolZone* zone) {
    m_assignedZone = zone;
}

PatrolZone* PoliceCar::getPatrolZone() const {
    return m_assignedZone;
}

bool PoliceCar::attemptRunOverPedestrian(Pedestrian& ped) {
    sf::Vector2f policePos = getPosition();
    sf::Vector2f npcPos = ped.getPosition();

    float distSq = (policePos.x - npcPos.x) * (policePos.x - npcPos.x) +
        (policePos.y - npcPos.y) * (policePos.y - npcPos.y);

    float combinedRadii = (m_sprite.getGlobalBounds().width / 2.f) + ped.getCollisionRadius();

    if (distSq < (combinedRadii * combinedRadii) * 0.8f) {
        ped.startBackingUp();
        ped.takeDamage(25);
        return true;
    }
    return false;
}

bool PoliceCar::readyForOfficerExit() const {
    return m_requestOfficerExit;
}

void PoliceCar::clearOfficerExitRequest() {
    m_requestOfficerExit = false;
}


bool PoliceCar::canSeePlayer(const Player& player, const std::vector<std::vector<sf::Vector2f>>& obstacles) {
    sf::Vector2f selfPos = getPosition(); // Vehicle::getPosition()
    sf::Vector2f playerPos = player.getPosition();
    sf::Vector2f directionToPlayer = playerPos - selfPos;
    float distanceToPlayer = std::hypot(directionToPlayer.x, directionToPlayer.y);

    // 1. Distance Check
    if (distanceToPlayer > m_visionDistance) {
        return false;
    }

    // 2. Field of View (FOV) Check
    if (distanceToPlayer > 0.001f) {
        // Vehicle's forward vector depends on its orientation.
        // Vehicle class has m_sprite.getRotation() and also `directionVec` for AI, and `angle` for player.
        // For AI driven, `directionVec` (if normalized) or angle from `m_sprite.getRotation()` can be used.
        // Let's use m_sprite.getRotation() consistent with Police.cpp
        // Assuming 0 rotation is UP for the sprite.
        float unitAngleRad = (getSprite().getRotation() - 90.f) * (static_cast<float>(M_PI) / 180.f);
        sf::Vector2f forwardVector(std::cos(unitAngleRad), std::sin(unitAngleRad));

        sf::Vector2f normalizedDirToPlayer = directionToPlayer / distanceToPlayer;

        float dotProduct = forwardVector.x * normalizedDirToPlayer.x + forwardVector.y * normalizedDirToPlayer.y;
        dotProduct = std::max(-1.0f, std::min(1.0f, dotProduct));
        float angleBetweenActualRad = std::acos(dotProduct);

        float fovThresholdRad = (m_fieldOfViewAngle / 2.f) * (static_cast<float>(M_PI) / 180.f);

        if (angleBetweenActualRad > fovThresholdRad) {
            return false;
        }
    }

    // 3. Line of Sight (LOS) Check - Simplified
    const int LOS_SAMPLE_POINTS = 3;
    for (int i = 0; i <= LOS_SAMPLE_POINTS; ++i) {
        if (i == 0 && LOS_SAMPLE_POINTS > 0) continue;
        sf::Vector2f testPoint = selfPos + directionToPlayer * (static_cast<float>(i) / LOS_SAMPLE_POINTS);
        for (const auto& polygon : obstacles) {
            if (CollisionUtils::pointInPolygon(testPoint, polygon)) {
                return false;
            }
        }
    }
    return true;
}

void PoliceCar::updateChaseBehavior(float dt, Player& player, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) {
    // If the car is not meant to be static (i.e., officer hasn't exited permanently),
    // ensure it tries to chase at its designated full speed.
    // The speed might have been temporarily reduced by a bump.
    if (!m_isStatic) {
        m_currentSpeed = m_speed; // Restore to full chasing speed
    }
    else {
        // If it's static (officer out), it shouldn't be chasing.
        // This state should ideally be prevented by the main update loop,
        // but as a safeguard:
        return;
    }

    m_repathTimer += dt;
    sf::Vector2f playerPosition = player.getPosition();

    bool needsNewPath = m_currentPath.empty() || m_currentPathIndex >= m_currentPath.size();
    if (!needsNewPath && m_currentTargetPosition.x != -1.f) {
        float dx = playerPosition.x - m_currentTargetPosition.x;
        float dy = playerPosition.y - m_currentTargetPosition.y;
        if ((dx * dx + dy * dy) > PLAYER_MOVE_THRESHOLD_FOR_REPATH_SQ) {
            needsNewPath = true;
        }
    }

    if (needsNewPath && m_repathTimer >= REPATH_COOLDOWN && PoliceManager::canRequestPath()) {
        PoliceManager::recordPathfindingCall();
        m_currentPath = m_pathfinder.findPath(getPosition(), playerPosition);
        m_currentPathIndex = 0;
        m_repathTimer = 0.f;
        if (!m_currentPath.empty()) {
            m_currentTargetPosition = playerPosition;
        }
        else {
            m_currentTargetPosition = sf::Vector2f(-1.f, -1.f); // Pathfinding failed
            // std::cout << "PoliceCar failed to find path to player." << std::endl;
        }
    }
    else if (needsNewPath && m_repathTimer >= REPATH_COOLDOWN) {
        m_repathTimer = 0.f; // Reset timer even if throttled, to try next frame
    }

    if (!m_currentPath.empty() && m_currentPathIndex < m_currentPath.size()) {
        sf::Vector2f nextWaypoint = m_currentPath[m_currentPathIndex];
        sf::Vector2f direction = nextWaypoint - getPosition();
        float distanceToWaypoint = std::hypot(direction.x, direction.y);

        if (distanceToWaypoint > 0.01f) {
            direction /= distanceToWaypoint; // Normalize
        }

        // --- Movement ---
        // This part needs to integrate with Vehicle's movement system if it exists,
        // or directly control m_sprite. For now, direct m_sprite control:
        sf::Vector2f currentPos = getPosition();
        float moveStep = m_currentSpeed * dt;
        sf::Vector2f nextPosCandidate = currentPos + direction * std::min(moveStep, distanceToWaypoint);

        // Basic future collision check (can be improved with Vehicle's system or CollisionUtils)
        bool collision = false;
        for (const auto& poly : blockedPolygons) {
            if (CollisionUtils::pointInPolygon(nextPosCandidate, poly)) { // A simple check
                collision = true;
                m_currentPath.clear(); // Collision, clear path to force repath
                m_currentPathIndex = 0;
                m_currentTargetPosition = sf::Vector2f(-1.f, -1.f);
                // std::cout << "PoliceCar: Collision with blocked polygon, clearing path." << std::endl;
                break;
            }
        }

        if (!collision) {
            setPosition(nextPosCandidate); // This should update Vehicle's position and ideally m_sprite
            // If PoliceCar directly manages m_sprite:
            m_sprite.setPosition(nextPosCandidate);

            // Update rotation
            if (distanceToWaypoint > 0.01f) { // Avoid atan2(0,0)
                float angle = std::atan2(direction.y, direction.x) * 180.f / M_PI;
                m_sprite.setRotation(angle + 90.f); // Adjust +90 if sprite faces up by default
            }
        }
        // --- End Movement ---


        if (distanceToWaypoint < TARGET_REACHED_DISTANCE) {
            m_currentPathIndex++;
            if (m_currentPathIndex >= m_currentPath.size()) {
                m_currentPath.clear(); // Reached end of path
            }
        }
    }
    else if (m_currentPath.empty()) {
        // No path, maybe move directly towards player if close enough and no obstacles?
        // Or just wait for pathfinder. For now, do nothing if no path.
    }
}

bool PoliceCar::attemptRunOverPlayer(Player& player, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) {
    sf::Vector2f policePos = getPosition();
    sf::Vector2f playerPos = player.getPosition();

    float distSq = (policePos.x - playerPos.x) * (policePos.x - playerPos.x) +
        (policePos.y - playerPos.y) * (policePos.y - playerPos.y);

    float combinedRadii = (m_sprite.getGlobalBounds().width / 2.f) + player.getCollisionRadius();

    if (distSq < (combinedRadii * combinedRadii) * 0.8f) {
        sf::Vector2f pushDir = playerPos - policePos;
        float length = std::sqrt(pushDir.x * pushDir.x + pushDir.y * pushDir.y);
        if (length != 0.f)
            pushDir /= length;

        float pushStrength = m_currentSpeed;//* 0.75f; // Increased knockback strength
        player.applyKnockback(pushDir * pushStrength, 0.4f);
        player.takeDamage(static_cast<int>(m_currentSpeed / 4.f)); // Damage calculation remains the same
        if (m_currentSpeed > 10.f)
            m_currentSpeed *= 0.6f;

        m_playerHitCount++; // Increment player-specific hit count
        if (m_playerHitCount >= 1) { // Officer exits after 1 hit
            m_requestOfficerExit = true;
        }
        // m_bumpCount++;

        return true;
    }

    return false;
}


void PoliceCar::draw(sf::RenderTarget& target) {
    // If Vehicle base class handles drawing, call Vehicle::draw(target);
    // Otherwise, draw m_sprite directly:
    target.draw(m_sprite);
}

// Implementing getPosition and setPosition to fulfill Vehicle virtuals if needed,
// and to ensure m_sprite is consistent with Vehicle's state.
sf::Vector2f PoliceCar::getPosition() const {
    // Prefer Vehicle's position if it's the source of truth
    // return Vehicle::getPosition(); 
    // If PoliceCar's m_sprite is the source of truth:
    return m_sprite.getPosition();
}

void PoliceCar::setPosition(const sf::Vector2f& pos) {
    // Vehicle::setPosition(pos); // Update base class state
    m_sprite.setPosition(pos);   // Update sprite position
}

// void PoliceCar::setTarget(const sf::Vector2f& target) {
// This might not be needed if updateChaseBehavior always targets the player.
// Could be used to set a temporary non-player target.
// }
