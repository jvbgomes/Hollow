#include "Entity.hpp"

Entity::Entity(float x, float y) {
    position = sf::Vector2f(x, y);
    velocity = sf::Vector2f(0.f, 0.f);
    sprite.setPosition(position);
}

sf::Vector2f Entity::getPosition() const {
    return position;
}

void Entity::setPosition(float x, float y) {
    position = sf::Vector2f(x, y);
    sprite.setPosition(position);
}