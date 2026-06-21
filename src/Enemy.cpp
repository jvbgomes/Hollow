#include "Enemy.hpp"

Enemy::Enemy(float x,float y, float speed , int health, float detectionRadius)
    : Entity(x, y), speed(speed), health(health), damage(1), detectionRadius(detectionRadius), alive(true) {
}

void Enemy::draw(sf::RenderWindow& window) {
        window.draw(sprite);
}

void Enemy::takeDamage(int amount) {
    health -= amount;
    if (health <= 0) {
        alive = false;
    }
}

bool Enemy::isAlive() const {
    return alive;
}