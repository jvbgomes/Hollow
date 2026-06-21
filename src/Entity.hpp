#pragma once

#include <SFML/Graphics.hpp>

class Map;

class Entity {
protected:
    sf::Vector2f position;
    sf::Vector2f velocity;
    sf::Sprite sprite;
    sf::Texture texture;

public:
    Entity(float x, float y);

    virtual void update(float deltaTime, const Map& map, sf::Vector2f playerPosition) = 0;
    virtual void draw(sf::RenderWindow& window) = 0;

    sf::Vector2f getPosition() const;
    void setPosition(float x, float y);
    sf::FloatRect getBounds() const;

    virtual ~Entity() = default;
};