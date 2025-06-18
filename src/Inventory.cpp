#include "Inventory.h"


Inventory::Inventory()
{
    // Items to pre-fill
    addItem("Health");
    addItem("Pistol");
    addItem("Ammo");
    addItem("SpeedBoost");
}

const std::unordered_map<std::string, int>& Inventory::getAllItems() const
{
    return items;
}


void Inventory::addItem(const std::string& name)
{
    items[name]++;
}

bool Inventory::useItem(const std::string& name)
{
    if (items[name] > 0) {
        items[name]--;
        return true;
    }
    return false;
}

int Inventory::getCount(const std::string& name) const
{
    auto it = items.find(name);
    return (it != items.end()) ? it->second : 0;
}
