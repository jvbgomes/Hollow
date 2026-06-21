#include "Boss.hpp"
#include "Map.hpp"
#include <cmath>

Boss::Boss(float x, float y)
    : Enemy(x, y, 80.f, 999, 9999.f), baseSpeed(80.f), speedTimer(0.f) {
    sf::Image img;
    img.create(32, 32, sf::Color(180, 0, 0));
    texture.loadFromImage(img);
    sprite.setTexture(texture);
    sprite.setOrigin(16.f, 16.f);
    sprite.setPosition(position);
}

void Boss::takeDamage(int amount) {
    // imortal — lamparinas de sal não têm efeito
}

void Boss::update(float deltaTime, const Map& map, sf::Vector2f playerPosition) {
    speedTimer += deltaTime;
    speed = baseSpeed + (speedTimer / 30.f) * 20.f;

    float dx = playerPosition.x - position.x;
    float dy = playerPosition.y - position.y;
    float distance = std::sqrt(dx * dx + dy * dy);

    if (distance > 0.f) {
        float nx = dx / distance;
        float ny = dy / distance;

        position.x += nx * speed * deltaTime;
        sprite.setPosition(position);
        if (map.isCollidingWith(sprite.getGlobalBounds())) {
            position.x -= nx * speed * deltaTime;
            sprite.setPosition(position);
        }

        position.y += ny * speed * deltaTime;
        sprite.setPosition(position);
        if (map.isCollidingWith(sprite.getGlobalBounds())) {
            position.y -= ny * speed * deltaTime;
            sprite.setPosition(position);
        }
    }
}
