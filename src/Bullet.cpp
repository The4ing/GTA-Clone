#include "Bullet.h"
#include "ResourceManager.h"
#include "CollisionUtils.h"

Bullet::Bullet() : active(false) {
    sprite.setTexture(ResourceManager::getInstance().getTexture("bullet"));
   // sprite.setPosition(position);
    sf::FloatRect bounds = sprite.getLocalBounds();
    sprite.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
    sprite.setScale(0.1, 0.1);
}

void Bullet::init(const sf::Vector2f& startPos, const sf::Vector2f& dir, float initialSpeed) {
    position = startPos;
    direction = dir; // Store the normalized direction
    speed = initialSpeed;
    sprite.setPosition(position);

    float angleRad = std::atan2(direction.y, direction.x);
    float angleDeg = angleRad * 180.f / 3.14159f;
    sprite.setRotation(angleDeg);
    // Sprite rotation could also be set here if bullets have a visual orientation based on direction
    // For a simple point bullet, rotation might not be visually necessary unless the sprite itself is directional
    // float angle = std::atan2(direction.y, direction.x) * 180.f / 3.14159f;
    // sprite.setRotation(angle);
    active = true;
}


void Bullet::update(float dt, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) {
    if (!active) return;
    move(direction, dt);
    sprite.setPosition(position);
}

void Bullet::draw(sf::RenderTarget& target) {
    if (!active) return;
    target.draw(sprite);
}

sf::Vector2f Bullet::getPosition() const {
    return position;
}

void Bullet::setPosition(const sf::Vector2f& pos) {
    position = pos;
    sprite.setPosition(position);
}

void Bullet::move(const sf::Vector2f& dir, float dt) {
    position += dir * speed * dt;
}

float Bullet::getSpeed() const {
    return speed;
}

bool Bullet::checkCollision(const std::vector<std::vector<sf::Vector2f>>& blockedPolygons,
    const std::vector<Pedestrian>& npcs,
    const std::vector<Vehicle>& cars) {
    if (!active) return false;

    for (const auto& polygon : blockedPolygons) {
        if (CollisionUtils::pointInPolygon(position, polygon)) {
            active = false;
            return true;
        }
    }

    for (const auto& npc : npcs) {
        if (sprite.getGlobalBounds().intersects(npc.getCollisionBounds())) {
            active = false;
            // npc.takeDamage(...) // ?????? ?? ????? ???
            return true;
        }
    }

    for (const auto& car : cars) {
        if (sprite.getGlobalBounds().intersects(car.getSprite().getGlobalBounds())) {
            active = false;
            // car.takeDamage(...) // ?????? ?? ????? ???
            return true;
        }
    }

    return false;
}

void Bullet::onCollision(GameObject& other) {
    // ????? ??dynamic_cast ?????? ???????? ??????? ??? ????? ????????
    if (Player* player = dynamic_cast<Player*>(&other)) {
        collideWithPlayer(*player);
    }
    else if (Present* present = dynamic_cast<Present*>(&other)) {
        collideWithPresent(*present);
    }
    // ???? ?????? ??????? ?????? ????? ?????
}

void Bullet::collideWithPlayer(Player& player) {
    // ?? ????? ?????? ???? ????? (??????)
    // player.takeDamage(10);
    setActive(false);
}

void Bullet::collideWithPresent(Present& present) {
    // ?? ????? ?????? ???? ????? (?? ???? ???????)
    setActive(false);
}

void Player::setCurrentWeapon(const std::string& name, CurrentWepapon Weapon, int maxAmmo) {
    m_currentWeaponName = name;
  //  m_currentWeaponAmmo = Weapon;
    m_maxWeaponAmmo = maxAmmo;
}

void Player::collideWithPlayer(Player& other) {
    // ????? ??? ?? ?????? ??????? ??? ??????
}

int Player::getWantedLevel() const {
    return m_wantedLevel;
}

int Player::getMoney() const {
    return m_money;
}

int Player::getHealth() const {
    return m_health;
}

int Player::getArmor() const {
    return m_armor;
}

std::string Player::getCurrentWeaponName() const {
    return m_currentWeaponName;
}

int Player::getMaxAmmo() const {
    return m_maxWeaponAmmo;
}
