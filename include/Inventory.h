#pragma once
#include <unordered_map>
#include <string>
#include <SFML/Graphics.hpp>

struct InventoryItem {
    int count = 0;
    const sf::Texture* texture = nullptr;  
    bool infinite = false;
};




class Inventory {
public:
    Inventory();

    void clearExceptFists();

    void addItem(const std::string& name, const sf::Texture& texture);
    bool useItem(const std::string& name);

    int getCount(const std::string& name) const;

    std::vector<std::string> getCollectedWeaponNames() const; // Added for stats

    const std::unordered_map<std::string, InventoryItem>& getAllItems() const;
    const sf::Texture* getItemTexture(const std::string& name) const;

    
private:
    std::unordered_map<std::string, InventoryItem> items;
};