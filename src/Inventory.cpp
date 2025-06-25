#pragma once
#include <unordered_map>
#include <string>
#include <SFML/Graphics.hpp>

struct InventoryItem {
    int count = 0;
    sf::Texture* texture = nullptr;
    bool infinite = false; 
};

class Inventory {
public:
    Inventory();

    void addItem(const std::string& name, sf::Texture* texture = nullptr);
    bool useItem(const std::string& name);
    int getCount(const std::string& name) const;
    const std::unordered_map<std::string, InventoryItem>& getAllItems() const;

    const sf::Texture* getItemTexture(const std::string& name) const;

private:
    std::unordered_map<std::string, InventoryItem> items;
};
