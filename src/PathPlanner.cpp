#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <SFML/System.hpp>
#include <cmath>
#include "QuadTree.h"
#include <iostream>
#include "CollisionUtils.h"

// Helper: hash for sf::Vector2i
namespace std {
    template<>
    struct hash<sf::Vector2i> {
        std::size_t operator()(const sf::Vector2i& v) const noexcept {
            return std::hash<int>()(v.x) ^ (std::hash<int>()(v.y) << 1);
        }
    };
}

inline bool closeEnough(float a, float b, float step) {
    return std::abs(a - b) < (step / 2.f);
}

std::vector<sf::Vector2f> findPathWorld(
    sf::Vector2f startPos,
    sf::Vector2f goalPos,
    float step,
    const QuadTree<std::vector<sf::Vector2f>>& blockedPolyTree,
    float radius,
    int maxNodes // *** ???? ***
) {
    auto toGrid = [step](sf::Vector2f pos) -> sf::Vector2i {
        return sf::Vector2i(static_cast<int>(pos.x / step), static_cast<int>(pos.y / step));
        };
    auto toWorld = [step](sf::Vector2i grid) -> sf::Vector2f {
        return sf::Vector2f(grid.x * step + step / 2.f, grid.y * step + step / 2.f);
        };

    sf::Vector2i start = toGrid(startPos);
    sf::Vector2i goal = toGrid(goalPos);

    struct Node {
        sf::Vector2i pos;
        float cost;
        float estimate;
        bool operator>(const Node& o) const { return (cost + estimate) > (o.cost + o.estimate); }
    };

    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> open;
    std::unordered_map<sf::Vector2i, sf::Vector2i> cameFrom;
    std::unordered_map<sf::Vector2i, float> costSoFar;
    std::unordered_set<sf::Vector2i> closed;

    open.push({ start, 0.f, float(std::hypot(goal.x - start.x, goal.y - start.y)) });
    costSoFar[start] = 0.f;

    const std::vector<sf::Vector2i> directions = {
        {1,0}, {-1,0}, {0,1}, {0,-1},
        {1,1}, {-1,1}, {1,-1}, {-1,-1}
    };

    int nodesExpanded = 0;

    while (!open.empty() && nodesExpanded < maxNodes) {
        Node current = open.top(); open.pop();
        if (closed.count(current.pos)) continue;
        closed.insert(current.pos);

        if (current.pos == goal ||
            (closeEnough(toWorld(current.pos).x, goalPos.x, step) && closeEnough(toWorld(current.pos).y, goalPos.y, step))) {
            break;
        }

        for (const auto& dir : directions) {
            sf::Vector2i next = current.pos + dir;
            sf::Vector2f nextWorld = toWorld(next);

            // ????? ?????? ???!
            if (nextWorld.x < 0 || nextWorld.y < 0 || nextWorld.x > 4640 || nextWorld.y > 4672)
                continue;

            // ????? ??????:
            sf::FloatRect area(nextWorld.x - radius, nextWorld.y - radius, radius * 2, radius * 2);
            auto nearby = blockedPolyTree.query(area);
            bool blocked = false;
            for (const auto& poly : nearby) {
                if (circleIntersectsPolygon(nextWorld, radius, poly)) {
                    blocked = true;
                    break;
                }
            }
            if (blocked) {
                std::cout << "[A*] Blocked node: (" << nextWorld.x << "," << nextWorld.y << ")\n";
            }


            float newCost = costSoFar[current.pos] + std::hypot(float(dir.x), float(dir.y));
            if (!costSoFar.count(next) || newCost < costSoFar[next]) {
                costSoFar[next] = newCost;
                float estimate = std::hypot(goal.x - next.x, goal.y - next.y);
                open.push({ next, newCost, estimate });
                cameFrom[next] = current.pos;
            }
        }
        nodesExpanded++;
    }

    std::vector<sf::Vector2f> path;
    sf::Vector2i end = goal;
    if (!cameFrom.count(end)) {
        float minDist = 1e9;
        for (const auto& pair : cameFrom) {
            float d = std::hypot(float(pair.first.x - goal.x), float(pair.first.y - goal.y));
            if (d < minDist) { minDist = d; end = pair.first; }
        }
        if (minDist > 2) return path;
    }

    for (sf::Vector2i at = end; at != start; at = cameFrom[at]) {
        path.push_back(toWorld(at));
    }
    path.push_back(toWorld(start));
    std::reverse(path.begin(), path.end());

    // ?????
    std::cout << "[A*] Expanded nodes: " << nodesExpanded << ", Path size: " << path.size() << std::endl;
    return path;
}
