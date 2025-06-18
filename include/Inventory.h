#pragma once
#include <unordered_map>
#include <string>

class Inventory {
private:
    std::unordered_map<std::string, int> items;

public:
    Inventory();

    const std::unordered_map<std::string, int>& getAllItems() const;

    void addItem(const std::string& name);

    bool useItem(const std::string& name);

    int getCount(const std::string& name) const;
};
