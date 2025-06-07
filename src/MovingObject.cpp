#include "MovingObject.h"





bool MovingObject::circleIntersectsPolygon(const sf::Vector2f& circleCenter, float radius, const std::vector<sf::Vector2f>& polygon) {
    size_t count = polygon.size();
    for (size_t i = 0; i < count; ++i) {
        sf::Vector2f p1 = polygon[i];
        sf::Vector2f p2 = polygon[(i + 1) % count];

        sf::Vector2f seg = p2 - p1;
        sf::Vector2f toCircle = circleCenter - p1;

        float segLengthSq = seg.x * seg.x + seg.y * seg.y;
        if (segLengthSq == 0.f)
            continue;

        float t = std::clamp((seg.x * toCircle.x + seg.y * toCircle.y) / segLengthSq, 0.f, 1.f);
        sf::Vector2f closest = p1 + t * seg;

        float dx = closest.x - circleCenter.x;
        float dy = closest.y - circleCenter.y;
        if ((dx * dx + dy * dy) < radius * radius)
            return true;
    }

    return false;
}