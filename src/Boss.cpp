#include "Boss.hpp"
#include "Map.hpp"
#include <cmath>

Boss::Boss(float x, float y) : Enemy(x, y, 50.f, 999, 9999.f), baseSpeed(50.f), speedTimer(0.f) {
    loadTextures("assets/maps/sprites/enemies/boss/", "boss");
    float scale = 40.f / 76.f;
    sprite.setScale(scale, scale);
    sprite.setOrigin(38.f, 38.f);
    sprite.setPosition(position);
}

void Boss::takeDamage(int) {
    // imortal — lamparinas de sal não têm efeito
}

void Boss::update(float dt, const Map& map, sf::Vector2f playerPos) {
    speedTimer += dt;
    speed = baseSpeed + (speedTimer / 30.f) * 20.f;

    float dx = playerPos.x - position.x;
    float dy = playerPos.y - position.y;
    float dist = std::sqrt(dx * dx + dy * dy);

    if (dist > 0.f) {
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
