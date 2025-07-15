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

std::vector<std::string> Inventory::getCollectedWeaponNames() const {
    std::vector<std::string> names;
    for (const auto& pair : items) {
        // Consider an item "collected" for stats if its count > 0 or it's an infinite item (like fists).
        // This assumes all items in inventory might be relevant for "collected weapons" list.
        // If specific categorization is needed (e.g. only actual weapons, not health packs),
        // InventoryItem would need a category, or names would need parsing.
        if (pair.second.count > 0 || pair.second.infinite) {
            // Exclude "Fists" if it's always there and not considered a "collected" weapon for stats
            if (pair.first == "Fists" && pair.second.infinite) {
                // Optionally skip fists or handle as a default, not "collected"
                // For now, let's include it to show something is always there.
            }
            names.push_back(pair.first);
        }
    }
    // std::sort(names.begin(), names.end()); // Optional: sort names alphabetically
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