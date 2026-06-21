#include "Shadow.hpp"
#include "Map.hpp"
#include <cmath>

Shadow::Shadow(float x, float y) 
    : Enemy(x, y, 60.f, 1, 150.f) {
    sf::Image img;
    img.create(24, 24, sf::Color(80, 0, 120));
    texture.loadFromImage(img);
    sprite.setTexture(texture);
    sprite.setOrigin(12.f, 12.f);
    sprite.setPosition(position);
}

void Shadow::update(float deltaTime, const Map& map, sf::Vector2f playerPosition) {
    float dx = playerPosition.x - position.x;
    float dy = playerPosition.y - position.y;
    float distance = std::sqrt(dx * dx + dy * dy);
  
    if (distance <= detectionRadius && distance > 0.f) {
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
