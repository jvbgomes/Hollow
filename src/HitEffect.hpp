#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

class HitEffect {
public:
    HitEffect(sf::Vector2f pos, sf::Color color);
    void update(float dt);
    void draw(sf::RenderTarget& target);
    bool isFinished() const;

private:
    struct Particle {
        sf::Vector2f pos;
        sf::Vector2f vel;
        float        life;
    };

    std::vector<Particle> m_particles;
    float     m_maxLife = 0.45f;
    sf::Color m_color;
};
