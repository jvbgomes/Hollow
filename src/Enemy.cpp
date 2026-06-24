#include "Enemy.hpp"
#include <cmath>

static const std::array<const char*, 8> DIR_NAMES = {"S","SE","L","NE","N","NO","O","SO"};

Enemy::Enemy(float x, float y, float spd, int hp, float det)
    : Entity(x, y), speed(spd), health(hp), damage(1),
      detectionRadius(det), alive(true), m_dir(Dir8::S) {}

void Enemy::loadTextures(const std::string& folder, const std::string& prefix) {
    for (int i = 0; i < 8; ++i)
        m_textures[i].loadFromFile(folder + prefix + "_" + DIR_NAMES[i] + ".png");
    sprite.setTexture(m_textures[0]);
}

Dir8 Enemy::calcDir(sf::Vector2f vel) const {
    float angle = std::atan2(vel.y, vel.x) * 180.f / 3.14159265f;
    if (angle < 0.f) angle += 360.f;
    int sector = static_cast<int>((angle + 22.5f) / 45.f) % 8;
    static const Dir8 map8[] = {
        Dir8::L, Dir8::SE, Dir8::S, Dir8::SO,
        Dir8::O, Dir8::NO, Dir8::N, Dir8::NE
    };
    return map8[sector];
}

void Enemy::applyDirection(sf::Vector2f vel) {
    if (vel.x == 0.f && vel.y == 0.f) return;
    m_dir = calcDir(vel);
    sprite.setTexture(m_textures[static_cast<int>(m_dir)]);
}

void Enemy::draw(sf::RenderTarget& target) {
    target.draw(sprite);
}

void Enemy::takeDamage(int amount) {
    health -= amount;
    if (health <= 0) alive = false;
}

bool Enemy::isAlive() const { return alive; }

sf::FloatRect Enemy::getBounds() const { return sprite.getGlobalBounds(); }

sf::FloatRect Enemy::getHitbox() const {
    sf::FloatRect b = sprite.getGlobalBounds();
    float shrinkX = b.width  * 0.35f;
    float shrinkY = b.height * 0.35f;
    return {b.left + shrinkX, b.top + shrinkY,
            b.width - shrinkX * 2.f, b.height - shrinkY * 2.f};
}
