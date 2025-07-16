#include "PoliceHelicopter.h"
#include "GameManager.h"
#include "Player.h"
#include "ResourceManager.h" // For textures
#include <cmath> // For std::hypot, std::atan2, M_PI
#include <iostream> // For debugging
#include "SoundManager.h"
#include <random>

PoliceHelicopter::PoliceHelicopter(GameManager& gameManager, const sf::Vector2f& startPosition)
    : m_gameManager(gameManager), m_targetPosition(startPosition), m_isRetreating(false),
     m_speed(60.f), m_rotationSpeed(90.f), m_health(200), m_altitude(100.f), m_fireCooldownTimer(0.f),
    needsCleanup(false)
{

    m_sprite.setTexture(ResourceManager::getInstance().getTexture("policeHelicopter")); // Placeholder texture name
    if (m_sprite.getTexture()) {
        m_sprite.setOrigin(m_sprite.getTexture()->getSize().x / 2.f, m_sprite.getTexture()->getSize().y / 2.f);
    }
    else {
        std::cerr << "PoliceHelicopter texture not loaded!" << std::endl;
        // Default origin if no texture, or handle error
    }
    m_sprite.setScale(0.25f, 0.25f); // Adjust as needed
    setPosition(startPosition);
    m_health = 200;
    m_speed = 60.f;
    m_engineSound.setBuffer(ResourceManager::getInstance().getSoundBuffer("helicopterMove"));
    m_engineSound.setLoop(true);
    SoundManager::getInstance().registerExternalSound(&m_engineSound);
    m_engineSound.play();
    // std::cout << "PoliceHelicopter spawned at: " << startPosition.x << ", " << startPosition.y << std::endl;
}
PoliceHelicopter::~PoliceHelicopter() {
    std::cout << "[PoliceHelicopter] Destroyed at " << this << std::endl;
    m_engineSound.stop();
    SoundManager::getInstance().unregisterExternalSound(&m_engineSound);
}

void PoliceHelicopter::update(float dt, Player& player, const std::vector<std::vector<sf::Vector2f>>& /*worldBoundaries*/) {
    if (!m_isRetreating) {
        m_targetPosition = player.getPosition();
        updateMovement(dt, m_targetPosition);
        updateAttackBehavior(dt, player);
    }
    else {
        updateMovement(dt, m_targetPosition);
        float dist = std::hypot(getPosition().x - m_targetPosition.x, getPosition().y - m_targetPosition.y);
        if (dist < 10.f) needsCleanup = true;
    }

    if (m_fireCooldownTimer > 0.f) {
        m_fireCooldownTimer -= dt;
    }
    float distance = std::hypot(getPosition().x - player.getPosition().x,
        getPosition().y - player.getPosition().y);
    float factor = 1.f - std::min(distance, 600.f) / 600.f;
    float baseVol = SoundManager::getInstance().isMuted() ? 0.f : SoundManager::getInstance().getVolume();
    m_engineSound.setVolume(baseVol * factor);
    // Rotor animation could be updated here
}

void PoliceHelicopter::update(float dt, const std::vector<std::vector<sf::Vector2f>>&) {
    // placeholder empty update if required by interface
}


void PoliceHelicopter::onCollision(GameObject& other) {
 
}


void PoliceHelicopter::collideWithPlayer(Player& player) {
  
}


void PoliceHelicopter::collideWithPresent(Present& present) {

}

void PoliceHelicopter::startRetreating(const sf::Vector2f& target) {
    m_isRetreating = true;
    m_targetPosition = target;
}

bool PoliceHelicopter::getNeedsCleanup() const
{
    return needsCleanup;
}

void PoliceHelicopter::setNeedsCleanup(bool change)
{
    needsCleanup = change;
}

void PoliceHelicopter::updateMovement(float dt, const sf::Vector2f& targetPosition) {
    sf::Vector2f currentPosition = getPosition();
    sf::Vector2f toPlayer = targetPosition - currentPosition;
    float distance = std::hypot(toPlayer.x, toPlayer.y);

    sf::Vector2f moveDir(0.f, 0.f);
    if (distance > IDEAL_DISTANCE_FROM_TARGET) {
        moveDir = toPlayer / distance;
    }
    else if (distance < IDEAL_DISTANCE_FROM_TARGET - 20.f && distance > 0.f) {
        moveDir = -toPlayer / distance;
    }

        // Rotate sprite to face movement direction or target
    if (moveDir.x != 0.f || moveDir.y != 0.f)
        move(moveDir, dt);

    float desiredAngle = std::atan2(toPlayer.y, toPlayer.x) * 180.f / M_PI + 90.f;
    float currentAngle = m_sprite.getRotation();
    float diff = desiredAngle - currentAngle;
    while (diff > 180.f) diff -= 360.f;
    while (diff < -180.f) diff += 360.f;
    float rotStep = m_rotationSpeed * dt;
    if (std::abs(diff) < rotStep) {
        m_sprite.setRotation(desiredAngle);
    }
    else {
        m_sprite.rotate(diff > 0 ? rotStep : -rotStep);
    }
}


void PoliceHelicopter::updateAttackBehavior(float dt, Player& player) {
    sf::Vector2f playerPosition = player.getPosition();
    float distanceToPlayer = std::hypot(getPosition().x - playerPosition.x, getPosition().y - playerPosition.y);

    if (distanceToPlayer <= SHOOTING_RANGE && m_fireCooldownTimer <= 0.f) {
        // std::cout << "Helicopter firing at player!" << std::endl;
        sf::Vector2f aimDir = playerPosition - getPosition();
        if (distanceToPlayer > 0) aimDir /= distanceToPlayer;
        std::uniform_real_distribution<float> spread(-0.15f, 0.15f);
        float angleOffset = spread(m_rng);
        float cosA = std::cos(angleOffset);
        float sinA = std::sin(angleOffset);
        sf::Vector2f rotated(cosA * aimDir.x - sinA * aimDir.y,
            sinA * aimDir.x + cosA * aimDir.y);
        aimDir = rotated;

        m_gameManager.addBullet(getPosition(), aimDir, BulletType::Pistol, false, true); // 'true' could indicate it's from an enemy / powerful source
        std::uniform_real_distribution<float> delay(-FIRE_RATE_VARIATION, FIRE_RATE_VARIATION);
        m_fireCooldownTimer = FIRE_RATE + delay(m_rng);
        SoundManager::getInstance().playSound("helicopterShot");
    }
}

void PoliceHelicopter::draw(sf::RenderTarget& target) {
    target.draw(m_sprite);
    // target.draw(m_rotorSprite);
}

sf::Vector2f PoliceHelicopter::getPosition() const {
    return m_sprite.getPosition();
}

void PoliceHelicopter::setPosition(const sf::Vector2f& pos) {
    m_sprite.setPosition(pos);
    // MovingObject::setPosition(pos); // If MovingObject base class tracks position
}

bool PoliceHelicopter::isRetreating() const
{
    return m_isRetreating;
}

bool PoliceHelicopter::isDestroyed() const {
    return m_health <= 0;
}

void PoliceHelicopter::takeDamage(int amount) {
    m_health -= amount;
    if (m_health < 0) m_health = 0;
    // std::cout << "Helicopter took " << amount << " damage, health: " << m_health << std::endl;
    if (isDestroyed()) {
        // std::cout << "Helicopter destroyed!" << std::endl;
        // Trigger explosion, sound effects etc.
        // m_gameManager.createExplosion(getPosition(), ExplosionSize::LARGE); // Example
    }
}

void PoliceHelicopter::move(const sf::Vector2f& direction, float dt) {
    sf::Vector2f newPos = getPosition() + direction * m_speed * dt;
    setPosition(newPos);
}

float PoliceHelicopter::getSpeed() const {
    return m_speed;
}

bool PoliceHelicopter::canSeePlayer(const Player& player, const std::vector<std::vector<sf::Vector2f>>& obstacles) {
    sf::Vector2f selfPos = getPosition();
    sf::Vector2f playerPos = player.getPosition();
    sf::Vector2f dir = playerPos - selfPos;
    float distance = std::hypot(dir.x, dir.y);
    const float VISION_DISTANCE = 500.f;
    const float FOV = 180.f;
    if (distance > VISION_DISTANCE)
        return false;
    if (distance > 0.001f) {
        float angleRad = (m_sprite.getRotation() - 90.f) * static_cast<float>(M_PI) / 180.f;
        sf::Vector2f forward(std::cos(angleRad), std::sin(angleRad));
        sf::Vector2f dirNorm = dir / distance;
        float dot = std::clamp(forward.x * dirNorm.x + forward.y * dirNorm.y, -1.f, 1.f);
        float angleBetween = std::acos(dot);
        if (angleBetween > (FOV / 2.f) * static_cast<float>(M_PI) / 180.f)
            return false;
    }
    const int LOS_POINTS = 3;
    for (int i = 1; i <= LOS_POINTS; ++i) {
        sf::Vector2f testPoint = selfPos + dir * (static_cast<float>(i) / LOS_POINTS);
        if (CollisionUtils::isInsideBlockedPolygon(testPoint, obstacles))
            return false;
    }
    return true;
}