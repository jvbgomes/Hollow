#pragma once

#include "Enemy.hpp"

class Shadow : public Enemy {
public:
    Shadow(float x, float y);

    void update(float deltaTime, const Map& map, sf::Vector2f playerPosition) override;
};