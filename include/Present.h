#pragma once

#include "GameObject.h"
#include <SFML/Graphics.hpp>
#include <string>

class Present : public GameObject {


public:
    Present(const sf::Texture& texture, const sf::Vector2f& pos);
    virtual ~Present() = default;

    // פעולות בסיסיות מה־GameObject
    void update(float dt, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) override;
    void draw(sf::RenderTarget& target) override;

    void onCollision(GameObject& other) ;
    void collideWithPlayer(Player& player) ;
    void collideWithPresent(Present& present) {} ;

    sf::Vector2f getPosition() const override;
    void setPosition(const sf::Vector2f& pos) override;

// לוגיקה שקשורה ל־Present
    void collect();
    bool isCollected() const;

    // פונקציות גישה ל־sprite
    sf::Sprite& getSprite();
    const sf::Sprite& getSprite() const;
    void setTexture(const sf::Texture& texture);

    // פונקציות וירטואליות שימומשו במחלקות יורשות
    virtual std::string getType() const = 0;
    virtual void applyEffect(class Player& player) = 0;
private:
    sf::Sprite sprite;
    sf::Vector2f position;
    bool collected = false;
    float respawnTimer = 0.f;
};
