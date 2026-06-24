#include "Projectile.hpp"
#include "Map.hpp"
#include <cmath>

Projectile::Projectile(sf::Vector2f startPos, sf::Vector2f direction)
    : position(startPos), maxRange(200.f), traveledDistance(0.f), active(true) {

    float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    if (length > 0.f)
        velocity = (direction / length) * 280.f;

    if (texture.loadFromFile("assets/sprites/items_lamp.png")) {
        sprite.setTexture(texture);
        sprite.setScale(0.5f, 0.5f); // 32x32 → 16x16 em voo
    } else {
        sf::Image img;
        img.create(12, 12, sf::Color(255, 200, 50));
        texture.loadFromImage(img);
        sprite.setTexture(texture);
    }
    sprite.setOrigin(sprite.getLocalBounds().width  / 2.f,
                     sprite.getLocalBounds().height / 2.f);
    sprite.setPosition(position);
}

void Projectile::update(float deltaTime, const Map& map) {
    if (!active) return;

    sf::Vector2f movement = velocity * deltaTime;
    position += movement;
    sprite.setPosition(position);

    traveledDistance += std::sqrt(movement.x * movement.x + movement.y * movement.y);
    sprite.rotate(360.f * deltaTime); // gira em voo

    if (traveledDistance >= maxRange || map.isCollidingWith(sprite.getGlobalBounds())) {
        active = false;
    }
}

void Projectile::draw(sf::RenderTarget& target) {
    if (!active) return;
    target.draw(sprite);
}

sf::FloatRect Projectile::getBounds() const {
    return sprite.getGlobalBounds();
}

bool Projectile::isActive() const {
    return active;
}

void Projectile::deactivate() {
    active = false;
}
