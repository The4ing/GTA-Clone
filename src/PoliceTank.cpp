#include "PoliceTank.h"
#include "GameManager.h"
#include "Player.h"
#include "ResourceManager.h" // For textures
#include "PoliceManager.h"   // For pathfinding throttle
#include "CollisionUtils.h"  // For basic collision checks
#include <cmath>
#include <iostream>

PoliceTank::PoliceTank(GameManager& gameManager, const sf::Vector2f& startPosition)
    : Vehicle(), // Call base Vehicle constructor
    m_gameManager(gameManager),
    m_pathfinder(*gameManager.getPathfindingGrid()),
    m_currentPathIndex(0),
    m_repathTimer(0.f),
    m_targetPosition(startPosition) {

    // Tank body sprite is assumed to be handled by Vehicle::sprite or Vehicle::setTexture
    // Example: this->setTexture(ResourceManager::getInstance().getTexture("police_tank_body"));
    // For now, we assume Vehicle's constructor or a derived method sets up the main sprite.
    // If Vehicle class doesn't have a sprite, you'd initialize m_bodySprite here.
    // We will use the Vehicle's sprite directly via getSprite() or by setting its texture.
    // For simplicity, let's assume Vehicle has setTexture and setOrigin
    Vehicle::setTexture(ResourceManager::getInstance().getTexture("policeTank")); // Placeholder
    Vehicle::getSprite().setOrigin(Vehicle::getSprite().getTextureRect().width / 2.f, Vehicle::getSprite().getTextureRect().height / 2.f); // Example
    Vehicle::setPosition(startPosition);

    m_turretSprite.setTexture(ResourceManager::getInstance().getTexture("tankBody")); // Placeholder
    if (m_turretSprite.getTexture()) {
        m_turretSprite.setOrigin(m_turretSprite.getTexture()->getSize().x / 4.f, m_turretSprite.getTexture()->getSize().y / 2.f); // Turret origin might be different
    }
    else {
        std::cerr << "PoliceTank turret texture not loaded!" << std::endl;
    }
    m_turretSprite.setScale(Vehicle::getSprite().getScale()); // Match body scale initially
    m_turretSprite.setPosition(startPosition);

    m_health = 500;
    m_speed = 30.f;
    // Vehicle::speed = m_speed; // If vehicle has a public speed member to override
    // std::cout << "PoliceTank spawned at: " << startPosition.x << ", " << startPosition.y << std::endl;
}

void PoliceTank::update(float dt, Player& player, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) {
    Vehicle::update(dt, blockedPolygons); // Base vehicle update (if any)

    updateAIBehavior(dt, player, blockedPolygons);
    aimAndFire(player, dt);

    if (m_cannonCooldownTimer > 0.f) {
        m_cannonCooldownTimer -= dt;
    }
    m_turretSprite.setPosition(getPosition()); // Keep turret on top of tank body
}

void PoliceTank::updateAIBehavior(float dt, Player& player, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) {
    m_repathTimer += dt;
    m_targetPosition = player.getPosition();

    bool needsNewPath = m_currentPath.empty() || m_currentPathIndex >= m_currentPath.size();
    if (!needsNewPath) {
        float dx = m_targetPosition.x - m_currentPath.back().x; // Check against last point of current path's target
        float dy = m_targetPosition.y - m_currentPath.back().y;
        // Using a simpler threshold for tanks as they are slower and paths might be shorter/more direct
        if ((dx * dx + dy * dy) > (PATHFINDING_GRID_SIZE * 4.0f) * (PATHFINDING_GRID_SIZE * 4.0f)) {
            needsNewPath = true;
        }
    }

    if (needsNewPath && m_repathTimer >= 2.0f && PoliceManager::canRequestPath()) { // Tanks repath less frequently
        PoliceManager::recordPathfindingCall();
        m_currentPath = m_pathfinder.findPath(getPosition(), m_targetPosition);
        m_currentPathIndex = 0;
        m_repathTimer = 0.f;
        if (m_currentPath.empty()) {
            // std::cout << "PoliceTank failed to find path." << std::endl;
        }
    }
    else if (needsNewPath && m_repathTimer >= 2.0f) {
        m_repathTimer = 0.f;
    }

    updateMovement(dt, blockedPolygons);
}

void PoliceTank::updateMovement(float dt, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) {
    if (!m_currentPath.empty() && m_currentPathIndex < m_currentPath.size()) {
        sf::Vector2f nextWaypoint = m_currentPath[m_currentPathIndex];
        sf::Vector2f directionToWaypoint = nextWaypoint - getPosition();
        float distanceToWaypoint = std::hypot(directionToWaypoint.x, directionToWaypoint.y);

        if (distanceToWaypoint > 1.0f) { // If not already at waypoint
            directionToWaypoint /= distanceToWaypoint; // Normalize

            // Tank rotation (body)
            float targetAngle = std::atan2(directionToWaypoint.y, directionToWaypoint.x) * 180.f / M_PI + 90.f;
            float currentAngle = Vehicle::getSprite().getRotation(); // Assuming Vehicle stores its sprite & rotation
            float angleDiff = targetAngle - currentAngle;
            // Normalize angle difference to [-180, 180]
            while (angleDiff > 180.f) angleDiff -= 360.f;
            while (angleDiff < -180.f) angleDiff += 360.f;

            float rotationThisFrame = m_rotationSpeed * dt;
            if (std::abs(angleDiff) < rotationThisFrame) {
                Vehicle::getSprite().setRotation(targetAngle);
            }
            else {
                Vehicle::getSprite().rotate(angleDiff > 0 ? rotationThisFrame : -rotationThisFrame);
            }

            // Move forward if mostly aligned
            if (std::abs(angleDiff) < 45.f) { // Only move if somewhat facing the target
                sf::Vector2f forwardVec(std::sin(currentAngle * M_PI / 180.f), -std::cos(currentAngle * M_PI / 180.f));
                sf::Vector2f newPos = getPosition() + forwardVec * m_speed * dt;

                // Basic collision for tanks (they might destroy some minor obstacles)
                bool collision = false;
                // For now, same simple collision as PoliceCar
                for (const auto& poly : blockedPolygons) {
                    if (CollisionUtils::pointInPolygon(newPos, poly)) {
                        collision = true;
                        m_currentPath.clear();
                        m_currentPathIndex = 0;
                        break;
                    }
                }
                if (!collision) {
                    Vehicle::setPosition(newPos);
                }
            }
        }

        if (distanceToWaypoint < PATHFINDING_GRID_SIZE * 0.75f) { // Reached waypoint
            m_currentPathIndex++;
            if (m_currentPathIndex >= m_currentPath.size()) {
                m_currentPath.clear();
            }
        }
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
        m_turretSprite.setRotation(targetTurretAngle);
    }
    else {
        m_turretSprite.rotate(turretAngleDiff > 0 ? turretRotationThisFrame : -turretRotationThisFrame);
    }

    // Firing logic
    if (distanceToPlayer <= CANNON_RANGE && std::abs(turretAngleDiff) < 10.f && m_cannonCooldownTimer <= 0.f) { // Turret aligned
        // std::cout << "Tank firing cannon!" << std::endl;
        sf::Vector2f cannonTipOffset(50, 0); // Placeholder offset from tank center to cannon tip, depends on sprite
        sf::Vector2f rotatedTip = sf::Vector2f(
            cannonTipOffset.x * std::cos((currentTurretAngle - 90.f) * M_PI / 180.f) - cannonTipOffset.y * std::sin((currentTurretAngle - 90.f) * M_PI / 180.f),
            cannonTipOffset.x * std::sin((currentTurretAngle - 90.f) * M_PI / 180.f) + cannonTipOffset.y * std::cos((currentTurretAngle - 90.f) * M_PI / 180.f)
        );
        sf::Vector2f projectileSpawnPos = getPosition() + rotatedTip;
        sf::Vector2f fireDirection = directionToPlayer / distanceToPlayer;

        // TODO: GameManager needs a method like addTankShell(position, direction)
        // m_gameManager.addTankShell(projectileSpawnPos, fireDirection); 
        // For now, let's use addBullet as a placeholder, assuming it can take a 'powerful' flag
        m_gameManager.addBullet(projectileSpawnPos, fireDirection, BulletType::TankShell); // ????
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
    m_turretSprite.setPosition(pos); // Keep turret synced
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
