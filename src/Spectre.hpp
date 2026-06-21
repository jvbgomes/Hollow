#pragma once

#include "Enemy.hpp"

class Spectre : public Enemy {
public:
    Spectre(float x, float y);

    void update(float deltaTime, const Map& map, sf::Vector2f playerPosition) override;
};
