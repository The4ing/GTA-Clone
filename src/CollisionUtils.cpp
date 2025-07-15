#include "CollisionUtils.h"

const QuadTree<std::vector<sf::Vector2f>>* CollisionUtils::s_blockedPolyTree = nullptr;

bool CollisionUtils::pointInPolygon(const sf::Vector2f& p, const std::vector<sf::Vector2f>& poly) {
    bool inside = false;
    size_t n = poly.size();

    for (size_t i = 0, j = n - 1; i < n; j = i++) {
        const sf::Vector2f& pi = poly[i];
        const sf::Vector2f& pj = poly[j];

        float denominator = pj.y - pi.y;
        if (std::abs(denominator) > 1e-6) { // Avoid division by zero
            bool intersect = ((pi.y > p.y) != (pj.y > p.y)) &&
                (p.x < (pj.x - pi.x) * (p.y - pi.y) / denominator + pi.x);
            if (intersect) {
                inside = !inside;
            }
        }
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

bool CollisionUtils::circleIntersectsPolygon(const sf::Vector2f& circleCenter, float radius, const std::vector<sf::Vector2f>& polygon) {
    // Check if circle center is inside polygon
    if (pointInPolygon(circleCenter, polygon))
        return true;

    // Check distance from circle to each edge
    size_t count = polygon.size();
    for (size_t i = 0; i < count; ++i) {
        const sf::Vector2f& p1 = polygon[i];
        const sf::Vector2f& p2 = polygon[(i + 1) % count];

        // Project circle center onto edge segment
        sf::Vector2f edge = p2 - p1;
        sf::Vector2f toCircle = circleCenter - p1;

        float edgeLengthSq = edge.x * edge.x + edge.y * edge.y;
        float t = std::clamp((edge.x * toCircle.x + edge.y * toCircle.y) / edgeLengthSq, 0.f, 1.f);
        sf::Vector2f closestPoint = p1 + edge * t;

        float dx = circleCenter.x - closestPoint.x;
        float dy = circleCenter.y - closestPoint.y;
        float distSq = dx * dx + dy * dy;

        if (distSq <= radius * radius)
            return true;
    }

    return false;
}

bool CollisionUtils::polygonIntersectsPolygon(const std::vector<sf::Vector2f>& polyA, const std::vector<sf::Vector2f>& polyB) {
    // משתמש באלגוריתם SAT (Separating Axis Theorem)
    auto project = [](const std::vector<sf::Vector2f>& poly, const sf::Vector2f& axis) -> std::pair<float, float> {
        float min = axis.x * poly[0].x + axis.y * poly[0].y;
        float max = min;
        for (const auto& p : poly) {
            float proj = axis.x * p.x + axis.y * p.y;
            if (proj < min) min = proj;
            if (proj > max) max = proj;
        }
        return { min, max };
        };

    auto getAxes = [](const std::vector<sf::Vector2f>& poly) {
        std::vector<sf::Vector2f> axes;
        for (size_t i = 0; i < poly.size(); ++i) {
            sf::Vector2f p1 = poly[i];
            sf::Vector2f p2 = poly[(i + 1) % poly.size()];
            sf::Vector2f edge = p2 - p1;
            axes.push_back(sf::Vector2f(-edge.y, edge.x)); // normal
        }
        return axes;
        };

    std::vector<sf::Vector2f> axesA = getAxes(polyA);
    std::vector<sf::Vector2f> axesB = getAxes(polyB);

    for (const auto& axis : axesA) {
        auto [minA, maxA] = project(polyA, axis);
        auto [minB, maxB] = project(polyB, axis);
        if (maxA < minB || maxB < minA)
            return false; // Separating axis found
    }

    for (const auto& axis : axesB) {
        auto [minA, maxA] = project(polyA, axis);
        auto [minB, maxB] = project(polyB, axis);
        if (maxA < minB || maxB < minA)
            return false;
    }

    return true; // No separating axis found → collision
}
