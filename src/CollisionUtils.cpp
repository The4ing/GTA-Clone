﻿#include "CollisionUtils.h"

const QuadTree<std::vector<sf::Vector2f>>* CollisionUtils::s_blockedPolyTree = nullptr;

bool CollisionUtils::pointInPolygon(const sf::Vector2f& p, const std::vector<sf::Vector2f>& poly) {
    bool inside = false;
    size_t n = poly.size();

    for (size_t i = 0, j = n - 1; i < n; j = i++) {
        const sf::Vector2f& pi = poly[i];
        const sf::Vector2f& pj = poly[j];   

        bool intersect = ((pi.y > p.y) != (pj.y > p.y)) &&
            (p.x < (pj.x - pi.x) * (p.y - pi.y) / (pj.y - pi.y + 0.0001f) + pi.x);
        if (intersect)
            inside = !inside;
    }

    return inside;
}

bool CollisionUtils::isInsideBlockedPolygon(const sf::Vector2f& point, const std::vector<std::vector<sf::Vector2f>>& polygons) {
    if (s_blockedPolyTree) {
        return CollisionUtils::isInsideBlockedPolygon(point, *s_blockedPolyTree);
    }
    for (const auto& poly : polygons) {
        if (CollisionUtils::pointInPolygon(point, poly)) {
            return true;
        }
    }
    return false;
}

bool CollisionUtils::isInsideBlockedPolygon(const sf::Vector2f& point) {
    if (s_blockedPolyTree) {
        return CollisionUtils::isInsideBlockedPolygon(point, *s_blockedPolyTree);
    }
    return false;
}

void CollisionUtils::setBlockedPolyTree(const QuadTree<std::vector<sf::Vector2f>>* tree) {
    s_blockedPolyTree = tree;
}
