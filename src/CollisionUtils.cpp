#include "CollisionUtils.h"

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
