#include "Projectile.hpp"
#include "Map.hpp"
#include <cmath>

Projectile::Projectile(sf::Vector2f startPos, sf::Vector2f direction)
    : position(startPos), maxRange(200.f), traveledDistance(0.f), active(true) {

    float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    if (length > 0.f) {
        velocity = (direction / length) * 300.f;
    }

    sf::Image img;
    img.create(12, 12, sf::Color(255, 200, 50));
    texture.loadFromImage(img);
    sprite.setTexture(texture);
    sprite.setOrigin(6.f, 6.f);
    sprite.setPosition(position);
}

void Projectile::update(float deltaTime, const Map& map) {
    if (!active) return;

    sf::Vector2f movement = velocity * deltaTime;
    position += movement;
    sprite.setPosition(position);

    traveledDistance += std::sqrt(movement.x * movement.x + movement.y * movement.y);

    if (traveledDistance >= maxRange || map.isCollidingWith(sprite.getGlobalBounds())) {
        active = false;
    }
}

void Projectile::draw(sf::RenderWindow& window) {
    if (!active) return;
    window.draw(sprite);
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
