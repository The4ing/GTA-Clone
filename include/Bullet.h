#pragma once

#include "MovingObject.h"
#include "Pedestrian.h"
#include "Vehicle.h"
#include "Player.h"
#include "Present.h"
#include "ResourceManager.h"
#include "CollisionUtils.h"
#include <SFML/Graphics.hpp>
#include <cmath>
#include "Police.h"
#include "GameObject.h"

enum class BulletType {
    Default,
    Pistol,
    Rifle,
    Minigun,
    Bazooka,
    Grenade,
    TankShell
};

class Bullet : public MovingObject {
public:
    Bullet();
    sf::Vector2f m_startPos;
    void init(const sf::Vector2f& startPos, const sf::Vector2f& direction,
        BulletType type = BulletType::Default, bool firedByPlayer = false,bool ignoreBlocked = false);

    void update(float dt, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) override;
    void draw(sf::RenderTarget& target) override;


    sf::Vector2f getPosition() const override;
    void setPosition(const sf::Vector2f& pos) override;
    void move(const sf::Vector2f& moveDir, float dt) override;
    void setActive(bool active);
    void setType(BulletType type);


    void onCollision(GameObject& other) override;
    void collideWithPlayer(Player& player) override;
    void collideWithPresent(Present& present) override;

    bool checkCollision(const std::vector<std::vector<sf::Vector2f>>& blockedPolygons,
        const std::vector<Pedestrian*>& npcs,
        const std::vector<Police*>& police,
        const std::vector<Vehicle*>& cars,
        Player& player);
    bool isActive() const;
  
    
   
    float getDamage() const;
    float getExplosionRadius() const;
    float getSpeed() const override;


private:
    sf::Sprite m_sprite;
    sf::Vector2f m_position;
    sf::Vector2f m_direction;
    BulletType m_type = BulletType::Default;

    float m_speed ;
    float m_damage;
    float m_explosionRadius ;

    bool m_active ;
    bool m_firedByPlayer ;
    bool m_ignoreBlocked ;

    void applyExplosionDamage(const std::vector<Pedestrian*>& npcs,
        const std::vector<Police*>& police,
        const std::vector<Vehicle*>& cars);
};