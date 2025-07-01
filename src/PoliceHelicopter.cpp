#include "PoliceHelicopter.h"
#include "GameManager.h"
#include "Player.h"
#include "ResourceManager.h" // For textures
#include <cmath> // For std::hypot, std::atan2, M_PI
#include <iostream> // For debugging

PoliceHelicopter::PoliceHelicopter(GameManager& gameManager, const sf::Vector2f& startPosition)
    : m_gameManager(gameManager), m_targetPosition(startPosition) {

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
    m_speed = 100.f;
    // std::cout << "PoliceHelicopter spawned at: " << startPosition.x << ", " << startPosition.y << std::endl;
}

void PoliceHelicopter::update(float dt, Player& player, const std::vector<std::vector<sf::Vector2f>>& /*worldBoundaries*/) {
    m_targetPosition = player.getPosition(); // Simple: always target player's current position

    updateMovement(dt, m_targetPosition);
    updateAttackBehavior(dt, player);

    if (m_fireCooldownTimer > 0.f) {
        m_fireCooldownTimer -= dt;
    }
    // Rotor animation could be updated here
}

void PoliceHelicopter::update(float dt, const std::vector<std::vector<sf::Vector2f>>&) {
    // placeholder empty update if required by interface
}


void PoliceHelicopter::onCollision(GameObject& other) {
    // ???? ???? ?????? ???? ?? ?? ???? ???? ????
}

// ???? ????
void PoliceHelicopter::collideWithPlayer(Player& player) {
    // ??????: ????? ??????? ?? ?????
}

// ???? ????
void PoliceHelicopter::collideWithPresent(Present& present) {
    // ?? ???? ????? ???? – ?? ???? ??? ??? ???? abstract
}


void PoliceHelicopter::updateMovement(float dt, const sf::Vector2f& targetPosition) {
    sf::Vector2f currentPosition = getPosition();
    sf::Vector2f directionToTarget = targetPosition - currentPosition;
    float distanceToTarget = std::hypot(directionToTarget.x, directionToTarget.y);

    if (distanceToTarget > 5.0f) {
        directionToTarget /= distanceToTarget; // Normalize

        // ????? ??move ????? ????? ????
        move(directionToTarget, dt);

        // Rotate sprite to face movement direction or target
        float angle = std::atan2(directionToTarget.y, directionToTarget.x) * 180.f / M_PI;
        m_sprite.setRotation(angle + 90.f); // Adjust if needed
    }
}


void PoliceHelicopter::updateAttackBehavior(float dt, Player& player) {
    sf::Vector2f playerPosition = player.getPosition();
    float distanceToPlayer = std::hypot(getPosition().x - playerPosition.x, getPosition().y - playerPosition.y);

    if (distanceToPlayer <= SHOOTING_RANGE && m_fireCooldownTimer <= 0.f) {
        // std::cout << "Helicopter firing at player!" << std::endl;
        sf::Vector2f aimDir = playerPosition - getPosition();
        if (distanceToPlayer > 0) aimDir /= distanceToPlayer;

        // TODO: Implement helicopter-specific projectile if different from normal bullets
        // For now, using GameManager's addBullet, assuming it's some kind of machine gun
        m_gameManager.addBullet(getPosition(), aimDir, BulletType::TankShell); // 'true' could indicate it's from an enemy / powerful source
        m_fireCooldownTimer = FIRE_RATE;

        // Sound effect for helicopter firing
        // m_gameManager.getSoundManager().playSound("helicopter_shoot"); // Example
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
