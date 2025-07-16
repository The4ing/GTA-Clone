// Store.h
#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include "Present.h"
#include "ResourceManager.h"
#include <functional>
#include "Player.h"
#include "Constants.h"

struct StoreItem {
    std::string name;
    int price ;
    sf::IntRect area; 
    std::function<void(Player&)> action; 
};

class Store {
public:
    Store(const sf::Vector2f& pos);
    void open(Player& player);
    void update(float dt);
   
    void drawUI(sf::RenderTarget& target);
   
    void handleInput(Player& player, const sf::RenderWindow& window);
    void setIsOpen(bool opt);
    const bool getPlayerClose() const;
    void setPlayerClose(bool opt);
    const sf::Vector2f getPosition() const ;
    std::optional<sf::Text> getHintTextIfClose() const;

private:
    std::vector<StoreItem> items;
    sf::Vector2f position;
    sf::Font font;
    bool isOpen;
    bool PlayerClose;
    bool mouseHeld;
    sf::Sprite icon;
    sf::Sprite Inv;
};
