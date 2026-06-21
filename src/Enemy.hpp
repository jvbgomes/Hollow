#pragma once

#include "Entity.hpp"

class Enemy : public Entity {
protected:
    float speed;
    int health;
    int damage;
    float detectionRadius;
    bool alive;

public:
    Enemy(float x, float y, float speed, int health, float detectionRadius);
    virtual ~Enemy() = default;
    
    void update(float deltaTime, const Map& map, sf::Vector2f playerPosition) override = 0;
    void draw(sf::RenderWindow& window) override;

    virtual void takeDamage(int amount = 1);
    bool isAlive() const;
};