#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <type_traits>
#include "QuadTree.h"

class CollisionUtils {
public:
    static bool pointInPolygon(const sf::Vector2f& point, const std::vector<sf::Vector2f>& polygon);
    static bool isInsideBlockedPolygon(const sf::Vector2f& point, const std::vector<std::vector<sf::Vector2f>>& polygons);
    static bool isInsideBlockedPolygon(const sf::Vector2f& point);
    static void setBlockedPolyTree(const QuadTree<std::vector<sf::Vector2f>>* tree);
    static bool circleIntersectsPolygon(const sf::Vector2f& circleCenter, float radius, const std::vector<sf::Vector2f>& polygon);
    static bool polygonIntersectsPolygon(const std::vector<sf::Vector2f>& polyA, const std::vector<sf::Vector2f>& polyB);

    template<typename T>
    static bool isInsideBlockedPolygon(const sf::Vector2f& point, const T& polyTree);
private:
    static const QuadTree<std::vector<sf::Vector2f>>* s_blockedPolyTree;
};

template<typename T>
bool CollisionUtils::isInsideBlockedPolygon(const sf::Vector2f& point, const T& polyTree) {
    sf::FloatRect query(point.x - 1.f, point.y - 1.f, 2.f, 2.f);
    std::vector<const std::vector<sf::Vector2f>*> possible;
    polyTree.query(query, possible);

    for (const auto* poly : possible) {
        if (CollisionUtils::pointInPolygon(point, *poly)) {
            return true;
        }
    }
    return false;
}
