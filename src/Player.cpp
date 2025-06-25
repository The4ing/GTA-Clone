#include "Player.h"
#include <SFML/Window/Keyboard.hpp>
#include "ResourceManager.h"
#include <cmath>
#include <algorithm> // for std::clamp
#include "CollisionUtils.h"
#include <iostream>
#include "Present.h"


Player::Player()
    : frameWidth(0), frameHeight(0), currentFrame(0),
    sheetCols(12), sheetRows(12), animTimer(0.f), animDelay(0.1f),
    m_money(0), m_health(MaxHealth), m_armor(100),
    m_currentWeaponName("Fists"), m_currentWeaponAmmo(Fists), m_maxWeaponAmmo(0),
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
    bool isHit = false; // החלף ללוגיקה שלך
    bool isDead = false; // החלף ללוגיקה שלך

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
                playAnimation("Pistol", false);
                //SoundManager::getInstance().playSound("gunshot"); 
                SoundManager::getInstance().playSound("guns");        }
            else if (m_currentWeaponName == "Rifle") {
                playAnimation("RifleShoot", false);
                SoundManager::getInstance().playSound("rifleShot");
            }
            else if (m_currentWeaponName == "Minigun") {
                playAnimation("MinigunShoot", false);
                SoundManager::getInstance().playSound("minigunShot"); //need to review
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
                SoundManager::getInstance().playSound("ThrowGrenade"); //need to review
            }
        }
        else if (shootingAnimPlaying) {
            // ממשיכים להנגן את אנימציית הירי עד לסיומה - לא עושים כלום כאן
        }
        else {
            // לולאת הליכה / עמידה רגילה
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

    animationManager->update(dt);
}





void Player::draw(sf::RenderTarget& window) {
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
    return { pos.x - 2.f, pos.y + 4.f }; // 🔁 שמאלה ולמטה
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

        //std::cout << "Healed from " << before << " to " << health << " HP.\n";
    }
    else {
        // אם החיים כבר מלאים – שמור את הפריט
        inventory.addItem("Health");
      //  std::cout << "Health is full! Added Health item to inventory.\n";
    }
}
void Player::increaseSpeed() {
    std::cout << speedBoostTimer << std::endl;
        if (speedBoostTimer <= 0.f) {
            speed += 30.f;
        speedBoostTimer = 15.f;
    }
    else {
        inventory.addItem("Speed");
    }
}
void Player::AddAmmo() {
    //bullets amount 
}

void Player::AddPistol() {
    //bullets amount 
}

void Player::UsingPistol()
{
  setCurrentWeapon("Pistol", Pistol, 0);
}

void Player::UsingFist()
{
    setCurrentWeapon("Fists", Fists, 0);
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
