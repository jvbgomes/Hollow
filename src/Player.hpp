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
    bool hasKeyItem;
    sf::Vector2f lastDirection;

public:
    Player(float x, float y);

    void update(float deltaTime, const Map& map, sf::Vector2f playerPosition) override;
    void draw(sf::RenderWindow& window) override;

    void handleInput();
    void takeDamage();

    int getHealth() const;
    int getDiaryPages() const;
    int getSaltLanterns() const;
    float getStamina() const;
    sf::Vector2f getDirection() const;
    bool hasKey() const;

    void addPage();
    void addLantern();
    void collectKey();
    bool useLantern();
};