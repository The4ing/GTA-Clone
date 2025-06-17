#pragma once
#include <vector>
#include <SFML/Graphics.hpp>
#include <type_traits>

template <typename T>
class QuadTree {
public:
    QuadTree(const sf::FloatRect& boundary, int capacity = 4, int level = 0, int maxLevel = 10)
        : boundary(boundary), capacity(capacity), level(level), maxLevel(maxLevel) {
    }

    QuadTree() : QuadTree(sf::FloatRect(0.f, 0.f, 4640.f, 4672.f)) {}

    bool insert(const sf::FloatRect& bounds, const T& item) {
        if (!boundary.intersects(bounds))
            return false;

        if (items.size() < capacity || level >= maxLevel) {
            items.push_back({ bounds, item });
            return true;
        }

        if (nodes.empty())
            subdivide();

        for (auto& node : nodes)
            if (node.insert(bounds, item))
                return true;

        return false;
    }

    // Returns a vector of items (by value)
    std::vector<T> query(const sf::FloatRect& range) const {
        std::vector<T> found;
        if (!boundary.intersects(range))
            return found;

        for (const auto& pair : items)
            if (range.intersects(pair.bounds))
                found.push_back(pair.data);

        if (!nodes.empty()) {
            for (const auto& node : nodes) {
                auto sub = node.query(range);
                found.insert(found.end(), sub.begin(), sub.end());
            }
        }

        return found;
    }

    // non-const version
    void query(const sf::FloatRect& range, std::vector<typename std::remove_pointer<T>::type*>& out) {
        if (!boundary.intersects(range))
            return;

        for (auto& pair : items)
            if (range.intersects(pair.bounds)) {
                if constexpr (std::is_pointer<T>::value)
                    out.push_back(pair.data);      // T is already a pointer (Vehicle*)
                else
                    out.push_back(&pair.data);     // T is value (RoadSegment)
            }

        if (!nodes.empty()) {
            for (auto& node : nodes)
                node.query(range, out);
        }
    }

    // const version
    void query(const sf::FloatRect& range, std::vector<const typename std::remove_pointer<T>::type*>& out) const {
        if (!boundary.intersects(range))
            return;

        for (const auto& pair : items)
            if (range.intersects(pair.bounds)) {
                if constexpr (std::is_pointer<T>::value)
                    out.push_back(pair.data);
                else
                    out.push_back(&pair.data);
            }

        if (!nodes.empty()) {
            for (const auto& node : nodes)
                node.query(range, out);
        }
    }

    void clear() {
        items.clear();
        nodes.clear();
    }

private:
    struct Item {
        sf::FloatRect bounds;
        T data;
    };

    sf::FloatRect boundary;
    int capacity;
    int level;
    int maxLevel;
    std::vector<Item> items;
    std::vector<QuadTree<T>> nodes;

    void subdivide() {
        float x = boundary.left;
        float y = boundary.top;
        float w = boundary.width / 2.f;
        float h = boundary.height / 2.f;

        nodes.emplace_back(sf::FloatRect(x, y, w, h), capacity, level + 1, maxLevel);
        nodes.emplace_back(sf::FloatRect(x + w, y, w, h), capacity, level + 1, maxLevel);
        nodes.emplace_back(sf::FloatRect(x, y + h, w, h), capacity, level + 1, maxLevel);
        nodes.emplace_back(sf::FloatRect(x + w, y + h, w, h), capacity, level + 1, maxLevel);
    }
};
