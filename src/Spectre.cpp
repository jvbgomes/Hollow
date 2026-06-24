#include "Spectre.hpp"
#include "Map.hpp"
#include <cmath>

Spectre::Spectre(float x, float y) : Enemy(x, y, 140.f, 2, 200.f) {
    loadTextures("assets/sprites/enemies/spectre/", "spectre");
    float scale = 32.f / 52.f;
    sprite.setScale(scale, scale);
    sprite.setOrigin(26.f, 26.f);
    sprite.setPosition(position);
}

void Spectre::update(float dt, const Map& map, sf::Vector2f playerPos) {
    float dx = playerPos.x - position.x;
    float dy = playerPos.y - position.y;
    float dist = std::sqrt(dx * dx + dy * dy);

    if (dist <= detectionRadius && dist > 0.f) {
        sf::Vector2f dir = { dx / dist, dy / dist };
        applyDirection(dir);

        position.x += dir.x * speed * dt;
        sprite.setPosition(position);
        if (map.isCollidingWith(sprite.getGlobalBounds())) {
            position.x -= dir.x * speed * dt;
            sprite.setPosition(position);
        }

        position.y += dir.y * speed * dt;
        sprite.setPosition(position);
        if (map.isCollidingWith(sprite.getGlobalBounds())) {
            position.y -= dir.y * speed * dt;
            sprite.setPosition(position);
        }
    }
}
