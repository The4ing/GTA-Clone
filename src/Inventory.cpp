#include "ResourceManager.h"
#include "Inventory.h"

Inventory::Inventory() {


    addItem("Fists", ResourceManager::getInstance().getTexture("Fists"));
    items["Fists"].infinite = true;
}

void Inventory::addItem(const std::string& name, const sf::Texture& texture) {
    auto it = items.find(name);
    if (it == items.end()) {
        items[name].count = 1;
        items[name].texture = &texture;

        
        if (name == "Pistol" || name == "Rifle" || name == "Minigun" ||
            name == "Bazooka" || name == "Knife" || name == "Grenade") {
            items[name].infinite = true;
        }
    }
    else {
        it->second.count++;
    }
}





bool Inventory::useItem(const std::string& name) {
    auto it = items.find(name);
    if (it != items.end()) {
        if (it->second.infinite) {
            return true; // ✅ תמיד מותר להשתמש
        }
        if (it->second.count > 0) {
            it->second.count--;
            return true;
        }
    }
    return false;
}

int Inventory::getCount(const std::string& name) const {
    auto it = items.find(name);
    return (it != items.end()) ? it->second.count : 0;
}

const std::unordered_map<std::string, InventoryItem>& Inventory::getAllItems() const {
    return items;
}

const sf::Texture* Inventory::getItemTexture(const std::string& name) const {
    auto it = items.find(name);  
    return (it != items.end()) ? it->second.texture : nullptr;
}

std::vector<std::string> Inventory::getCollectedWeaponNames() const {
    std::vector<std::string> names;
    for (const auto& pair : items) {
        if (pair.second.count > 0 || pair.second.infinite) {
            // Exclude "Fists" if it's always there and not considered a "collected" weapon for stats
            if (pair.first == "Fists" && pair.second.infinite) {
            }
            names.push_back(pair.first);
        }
    }
    return names;
}


void Inventory::clearExceptFists() {
    for (auto it = items.begin(); it != items.end();) {
        if (it->first != "Fists") {
            it = items.erase(it);
        }
        else {
            ++it;
        }
    }
    items["Fists"].count = 1;
    items["Fists"].infinite = true;
}