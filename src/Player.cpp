#include "Player.h"
#include <SFML/Window/Keyboard.hpp>
#include "ResourceManager.h"
#include <cmath>
#include <algorithm> // for std::clamp
#include "CollisionUtils.h"
#include <iostream>
#include "Present.h"
#include "Vehicle.h" 

Player::Player()
    : m_currentVehicle(nullptr), frameWidth(0), frameHeight(0), currentFrame(0),
    sheetCols(12), sheetRows(12), animTimer(0.f), animDelay(0.1f),
    m_money(0), m_health(MaxHealth), m_armor(100),
    m_currentWeaponName("Fists"), m_currentWeaponAmmo(W_Fists), m_maxWeaponAmmo(0),
    m_wantedLevel(6)
{
    sf::Texture& texture = ResourceManager::getInstance().getTexture("player");
    sprite.setTexture(texture);

    int sheetW = texture.getSize().x;
    int sheetH = texture.getSize().y;
    frameWidth = sheetW / sheetCols;
    frameHeight = sheetH / sheetRows;

    sprite.setOrigin(frameWidth / 2.f, frameHeight / 2.f);
    sprite.setScale(0.25f, 0.25f);
    sprite.setTextureRect(sf::IntRect(0, 0, frameWidth, frameHeight));

    position = { 100.f, 100.f };
    sprite.setPosition(position);
    sprite.setScale(0.07, 0.07);

    animationManager = std::make_unique<AnimationManager>(sprite, frameWidth, frameHeight, sheetCols, sheetRows);
    animationManager->initAnimations();
    
    WeaponsAmmo = {
     {"Fists",   AmmoSetting{}},
     {"Pistol",  AmmoSetting{}},
     {"Rifle",   AmmoSetting{}},
     {"Minigun", AmmoSetting{}},
     {"Bazooka", AmmoSetting{}},
     {"Knife",   AmmoSetting{}},
     {"Grenade", AmmoSetting{}}
    };
}

void Player::setPosition(const sf::Vector2f& pos) {
    sprite.setPosition(pos);
}

sf::Vector2f Player::getPosition() const {
    return sprite.getPosition();
}



void Player::takeDamage(int amount)
{
}




void Player::update(float dt, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) {
    if (m_currentVehicle) {
        // שחקן ברכב - מעביר שליטה לרכב
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
            m_currentVehicle->accelerate(dt);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
            m_currentVehicle->brake(dt);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
            m_currentVehicle->steerLeft(dt);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
            m_currentVehicle->steerRight(dt);

        // סנכרון מיקום השחקן עם הרכב
        setPosition(m_currentVehicle->getPosition());
    }
    else {
        // שחקן הולך ברגל
        sf::Vector2f movement(0.f, 0.f);

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
            movement.x -= 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
            movement.x += 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
            movement.y -= 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
            movement.y += 1.f;

        bool isMoving = (movement.x != 0.f || movement.y != 0.f);
        bool isShooting = sf::Keyboard::isKeyPressed(sf::Keyboard::Space);
        bool isHit = false;
        bool isDead = false;

        if (speedBoostTimer > 0.f)
            speedBoostTimer -= dt;
        else
            speed = BasicSpeed;

        if (isMoving) {
            float len = std::sqrt(movement.x * movement.x + movement.y * movement.y);
            if (len != 0.f)
                movement /= len;

            float angle = std::atan2(movement.y, movement.x) * 180.f / 3.14159f + 90.f;
            sprite.setRotation(angle);

            sf::Vector2f nextPos = sprite.getPosition() + movement * speed * dt;

            bool collision = false;
            for (const auto& poly : blockedPolygons) {
                if (CollisionUtils::pointInPolygon(nextPos, poly)) {
                    collision = true;
                    break;
                }
            }

            if (!collision) {
                sprite.move(movement * speed * dt);
            }
        }

        bool shootingAnimPlaying = false;
        if (!currentAnimationName.empty()) {
            bool isShootAnim = currentAnimationName.find("Throw") != std::string::npos ||
                currentAnimationName.find("Shoot") != std::string::npos ||
                currentAnimationName.find("Attack") != std::string::npos;
            shootingAnimPlaying = isShootAnim && !animationManager->isAnimationFinished();
        }

        if (isDead) {
            playAnimation("HurtDie");
        }
        else if (isHit) {
            playAnimation("Hurt");
        }
        else {
            if (isShooting && !shootingAnimPlaying) {
                if (m_currentWeaponName == "Fists") {
                    playAnimation("BatAttack", false);
                }
                else if (m_currentWeaponName == "Pistol") {
                    playAnimation("PistolShoot", false);
                    SoundManager::getInstance().playSound("gunshot");
                }
                else if (m_currentWeaponName == "Rifle") {
                    playAnimation("RifleShoot", false);
                    SoundManager::getInstance().playSound("rifleShot");
                }
                else if (m_currentWeaponName == "Minigun") {
                    playAnimation("MinigunShoot", false);
                    SoundManager::getInstance().playSound("minigunShot");
                }
                else if (m_currentWeaponName == "Bazooka") {
                    playAnimation("BazookaShoot", false);
                    SoundManager::getInstance().playSound("RPGshot");
                }
                else if (m_currentWeaponName == "Knife") {
                    playAnimation("KnifeAttack", false);
                    SoundManager::getInstance().playSound("KnifeAttack");
                }
                else if (m_currentWeaponName == "Grenade") {
                    playAnimation("ThrowGrenade", false);
                    SoundManager::getInstance().playSound("ThrowGrenade");
                }
            }
            else if (shootingAnimPlaying) {
                // המשך ניגון של אנימציית ירי - לא עושים כלום
            }
            else {
                if (m_currentWeaponName == "Fists") {
                    if (isMoving)
                        playAnimation("Idle_NoWeapon");
                    else {
                        setSpecificFrame(0, 0);
                        return;
                    }
                }
                else if (m_currentWeaponName == "Pistol") {
                    if (isMoving)
                        playAnimation("PistolWalk");
                    else {
                        setSpecificFrame(6, 10);
                        return;
                    }
                }
                else if (m_currentWeaponName == "Rifle") {
                    if (isMoving)
                        playAnimation("RifleWalk");
                    else {
                        setSpecificFrame(5, 5);
                        return;
                    }
                }
                else if (m_currentWeaponName == "Minigun") {
                    if (isMoving)
                        playAnimation("MinigunWalk");
                    else {
                        setSpecificFrame(8, 1);
                        return;
                    }
                }
                else if (m_currentWeaponName == "Bazooka") {
                    if (isMoving)
                        playAnimation("BazookaWalk");
                    else {
                        setSpecificFrame(9, 9);
                        return;
                    }
                }
                else if (m_currentWeaponName == "Knife") {
                    playAnimation("Idle_NoWeapon");
                }
                else if (m_currentWeaponName == "Grenade") {
                    playAnimation("Idle_NoWeapon");
                }
            }
        }
    }

    animationManager->update(dt);
}






void Player::draw(sf::RenderTarget& window) {
    if (m_currentVehicle) {
            // Optionally, don't draw player sprite, or draw a different "in car" sprite
            // For now, we'll just not draw the player if they are in a vehicle,
            // assuming the vehicle itself will be drawn by CarManager.
            return;
    }
    sf::CircleShape circle(getCollisionRadius());
    circle.setOrigin(getCollisionRadius(), getCollisionRadius());
    circle.setPosition(getCenter());
    circle.setFillColor(sf::Color::Transparent);
    circle.setOutlineColor(sf::Color::Blue);
    circle.setOutlineThickness(1.f);
    window.draw(circle);


    window.draw(sprite);
}

sf::FloatRect Player::getCollisionBounds(const sf::Vector2f& offset) const {
    // Get the center position of the sprite (already scaled)
    if (m_currentVehicle) {
        
            // If in vehicle, collision bounds are effectively the vehicle's bounds
            // This might need to be handled by the Vehicle class or GameManager
            return {}; // Return empty rect, or vehicle's bounds
    }
    sf::Vector2f pos = sprite.getPosition();
    sf::Vector2f size(frameWidth * sprite.getScale().x, frameHeight * sprite.getScale().y);

    return {
        pos.x - size.x / 2.f + offset.x,
        pos.y - size.y / 2.f + offset.y,
        size.x,
        size.y
    };
}

sf::Vector2f Player::getCenter() const {
    sf::Vector2f pos = sprite.getPosition();
    return { pos.x - 2.f, pos.y + 4.f };
}

float Player::getCollisionRadius() const {
    // Set to match visual size — adjust as needed
    return 6.f; // in pixels
}


void Player::onCollision(GameObject& other) {
    other.collideWithPlayer(*this); // ← הפוך: אתה Player, שולח את עצמך
}

void Player::collideWithPresent(Present& present) {
    /*if (!present.isCollected()) {
        present.applyEffect(*this);
        present.collect();
    }*/ 
   // std::cout << "collide with present" << std::endl;
}


Inventory& Player::getInventory()
{
    return inventory; // ← מחזיר הפניה!
}

const Inventory& Player::getInventory() const
{
    return inventory;
}

void Player::heal(int amount) {
    if (m_health < MaxHealth) {
        m_health += amount;
        if (m_health > MaxHealth)
            m_health = MaxHealth;

    }
    else {
        
        inventory.addItem("Health", ResourceManager::getInstance().getTexture("Health"));
     
    }
}
void Player::increaseSpeed() {
    std::cout << speedBoostTimer << std::endl;
        if (speedBoostTimer <= 0.f) {
            speed += 30.f;
        speedBoostTimer = 15.f;
    }
    else {
        inventory.addItem("Speed", ResourceManager::getInstance().getTexture("Speed"));
    }
}
void Player::AddAmmo() {
    
}

void Player::AddWeapon(const std::string name) {
    inventory.addItem(name, ResourceManager::getInstance().getTexture(name));
}



void Player::playAnimation(const std::string& animName, bool loop, bool pingpong) {
    if (!animationManager) return;
    if (currentAnimationName != animName) {
        animationManager->setAnimation(animName, loop, pingpong);
        currentAnimationName = animName;
    }
}

// להציג פריים ספציפי בספרייט שיט
void Player::setSpecificFrame(int row, int col) {
    sf::IntRect rect(col * frameWidth, row * frameHeight, frameWidth, frameHeight);
    sprite.setTextureRect(rect);
}


void Player::enterVehicle(Vehicle* vehicle) {
   
        m_currentVehicle = vehicle;
    // Player sprite could be hidden here, or its position updated to match vehicle
    // For now, Player::draw() handles not drawing the player.
    // Player::update() will sync position.
}

void Player::exitVehicle() {
    if (m_currentVehicle) {
        // Position the player slightly beside the vehicle when exiting
        // This is a simple offset; might need adjustment based on vehicle size/orientation
        sf::Vector2f vehiclePos = m_currentVehicle->getPosition();
        // TODO: Get vehicle's current orientation to place player appropriately (e.g., beside a door)
        // For now, simple offset
        setPosition(sf::Vector2f(vehiclePos.x + 10.f, vehiclePos.y));
        m_currentVehicle = nullptr;
    }
}

Vehicle* Player::getCurrentVehicle() const {
    return m_currentVehicle;
}

bool Player::isInVehicle() const {
    return m_currentVehicle != nullptr;
}