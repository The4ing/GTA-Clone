#include "PoliceCar.h"
#include "ResourceManager.h"
#include "GameManager.h" // For PathfindingGrid, Player access
#include "Player.h"      // For Player object
#include "PoliceManager.h" // For canRequestPath, recordPathfindingCall
#include "CollisionUtils.h" // For collision checks if needed
#include <cmath>        // For std::hypot, std::atan2, M_PI
#include <iostream>     // For debugging

PoliceCar::PoliceCar(GameManager& gameManager, const sf::Vector2f& startPosition)
    : Vehicle(), // Call base Vehicle constructor
    m_gameManager(gameManager),
    m_pathfinder(*gameManager.getPathfindingGrid()), // Initialize Pathfinder
    m_currentPathIndex(0),
    m_repathTimer(0.f),
    m_currentTargetPosition(-1.f, -1.f),
    m_isAmbient(true), // Explicitly initialize, though defaults in .h
    m_playerCausedWantedIncrease(false) // Explicitly initialize
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
}

void PoliceCar::setIsAmbient(bool isAmbient) {
    m_isAmbient = isAmbient;
}

bool PoliceCar::isAmbient() const {
    return m_isAmbient;
}

void PoliceCar::update(float dt, Player& player, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) {
    // Always call Vehicle::update for basic movement processing, collision response from Vehicle side etc.
    Vehicle::update(dt, blockedPolygons);

    if (m_isAmbient && player.getWantedLevel() >= 3) {
        m_isAmbient = false; // Transition to non-ambient (aggressive)
        // No need to reset m_playerCausedWantedIncrease here, that's for a different mechanic
    }

    if (m_isAmbient) {
        // Ambient behavior:
        // For now, Vehicle::update() handles its general movement if it's on a path (e.g., Bezier curve).
        // If not on a path, it might just continue straight or stop, depending on Vehicle's AI.
        // No specific chase behavior.
        // We might need to add more sophisticated ambient driving logic here later if Vehicle::update
        // isn't sufficient for believable ambient police car driving.
    }
    else {
        // Non-ambient (aggressive) behavior:
        updateChaseBehavior(dt, player, blockedPolygons);

        // Basic collision with player for "run over" - can be expanded
        if (m_bumpCooldown <= 0.f && attemptRunOverPlayer(player)) {
            player.takeDamage(25); // Example damage
            m_bumpCooldown = 3.f; // Cooldown for bumping/damaging player
            // Potentially add some knockback or effect
        }
    }

    if (m_bumpCooldown > 0.f) {
        m_bumpCooldown -= dt;
    }
}

void PoliceCar::updateChaseBehavior(float dt, Player& player, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) {
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
        float moveStep = m_speed * dt;
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

bool PoliceCar::attemptRunOverPlayer(Player& player) {
    sf::Vector2f policePos = getPosition();
    sf::Vector2f playerPos = player.getPosition();

    float distSq = (policePos.x - playerPos.x) * (policePos.x - playerPos.x) +
        (policePos.y - playerPos.y) * (policePos.y - playerPos.y);

    float combinedRadii = (m_sprite.getGlobalBounds().width / 2.f) + player.getCollisionRadius();

    if (distSq < (combinedRadii * combinedRadii) * 0.8f) {
        // ?????! ???? ?? ?????
        sf::Vector2f pushDir = playerPos - policePos;
        float length = std::sqrt(pushDir.x * pushDir.x + pushDir.y * pushDir.y);
        if (length != 0.f)
            pushDir /= length;

        float pushStrength = 45.f; // ??? ??????? ????? – ???? ????? ?????

        player.setPosition(playerPos + pushDir * pushStrength);
        player.takeDamage(25); // ?? ?? ??? ???

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
