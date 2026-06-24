#pragma once
#include "Entity.hpp"
#include <array>

enum class Dir8 { S=0, SE=1, L=2, NE=3, N=4, NO=5, O=6, SO=7 };

class Enemy : public Entity {
protected:
    float  speed;
    int    health;
    int    damage;
    float  detectionRadius;
    bool   alive;
    Dir8   m_dir;

    std::array<sf::Texture, 8> m_textures;

    void loadTextures(const std::string& folder, const std::string& prefix);
    void applyDirection(sf::Vector2f vel);
    Dir8 calcDir(sf::Vector2f vel) const;

public:
    Enemy(float x, float y, float speed, int health, float detectionRadius);
    virtual ~Enemy() = default;

    void update(float dt, const Map& map, sf::Vector2f playerPos) override = 0;
    void draw(sf::RenderTarget& target) override;

    virtual void takeDamage(int amount = 1);
    bool isAlive() const;
    sf::FloatRect getBounds() const;
};
