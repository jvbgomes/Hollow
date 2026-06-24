#pragma once

#include <SFML/Graphics.hpp>

class Map;

class Projectile {
private:
    sf::Sprite sprite;
    sf::Texture texture;
    sf::Vector2f position;
    sf::Vector2f velocity;
    float maxRange;
    float traveledDistance;
    bool active;

public:
    Projectile(sf::Vector2f startPos, sf::Vector2f direction);

    void update(float deltaTime, const Map& map);
    void draw(sf::RenderTarget& target);

    sf::FloatRect getBounds() const;
    bool isActive() const;
    void deactivate();
};
