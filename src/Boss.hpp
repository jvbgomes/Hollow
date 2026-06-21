#pragma once

#include "Enemy.hpp"

class Boss : public Enemy {
private:
    float baseSpeed;
    float speedTimer;

public:
    Boss(float x, float y);

    void update(float deltaTime, const Map& map, sf::Vector2f playerPosition) override;
    void takeDamage(int amount = 1) override;
};
