﻿#pragma once

#include "GameObject.h"
#include <SFML/Graphics.hpp>
#include <string>

class Present : public GameObject {


public:
    Present(const sf::Texture& texture, const sf::Vector2f& pos);
    virtual ~Present() = default;

    void update(float dt, const std::vector<std::vector<sf::Vector2f>>& blockedPolygons) override;
    void draw(sf::RenderTarget& target) override;

    void onCollision(GameObject& other) ;
    void collideWithPlayer(Player& player) ;
    void collideWithPresent(Present& present) {} ;

    sf::Vector2f getPosition() const override;
    void setPosition(const sf::Vector2f& pos) override;

    void collect();
    bool isCollected() const;

    sf::Sprite& getSprite();
    const sf::Sprite& getSprite() const;
    void setTexture(const sf::Texture& texture);

    virtual std::string getType() const = 0;
    virtual void applyEffect(class Player& player) = 0;
private:
    sf::Sprite sprite;
    sf::Vector2f position;
    bool collected = false;
    float respawnTimer = 0.f;
};
