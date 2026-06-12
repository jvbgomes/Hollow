#pragma once

#include "Entity.hpp"

class Player : public Entity {
private:
    float speed;
    float stamina;
    float maxStamina;
    bool isSprinting;
    int health;
    int diaryPages;
    int saltLanterns;

public:
    Player(float x, float y);

    void update(float deltaTime, const Map& map) override;
    void draw(sf::RenderWindow& window) override;

    void handleInput();
    void takeDamage();

    int getHealth() const;
    int getDiaryPages() const;
    int getSaltLanterns() const;
    float getStamina() const;
};