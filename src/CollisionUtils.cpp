#include "CollisionUtils.h"
#include <cmath>

// פונקציית עזר לחישוב מרחק בין שתי נקודות
float distance(const sf::Vector2f& a, const sf::Vector2f& b) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return std::sqrt(dx * dx + dy * dy);
}

// בדיקה אם מעגל חותך מצולע
bool circleIntersectsPolygon(const sf::Vector2f& circleCenter, float radius, const std::vector<sf::Vector2f>& polygon) {
    for (size_t i = 0; i < polygon.size(); ++i) {
        sf::Vector2f a = polygon[i];
        sf::Vector2f b = polygon[(i + 1) % polygon.size()];

        // הקרנה של מרכז המעגל על קטע
        sf::Vector2f ab = b - a;
        sf::Vector2f ac = circleCenter - a;
        float abLenSq = ab.x * ab.x + ab.y * ab.y;
        float proj = (ac.x * ab.x + ac.y * ab.y) / abLenSq;

        proj = std::fmax(0.f, std::fmin(1.f, proj));
        sf::Vector2f closest = a + proj * ab;

        if (distance(closest, circleCenter) <= radius)
            return true;
    }

    return false;
}
