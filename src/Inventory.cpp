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
        items[name].texture = &texture; // 🟢 עובד, כי הטיפוס תואם
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
    auto it = items.find(name);  // ?? ????? ???
    return (it != items.end()) ? it->second.texture : nullptr;
}
