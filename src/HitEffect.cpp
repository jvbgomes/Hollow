#include "HitEffect.hpp"
#include <cmath>
#include <cstdlib>

static float randF(float lo, float hi) {
    return lo + (hi - lo) * (std::rand() / (float)RAND_MAX);
}

HitEffect::HitEffect(sf::Vector2f pos, sf::Color color) : m_color(color) {
    const int COUNT = 10;
    for (int i = 0; i < COUNT; ++i) {
        float angle = randF(0.f, 6.2832f);
        float speed = randF(40.f, 120.f);
        Particle p;
        p.pos  = pos;
        p.vel  = { std::cos(angle) * speed, std::sin(angle) * speed };
        p.life = m_maxLife;
        m_particles.push_back(p);
    }
}

void HitEffect::update(float dt) {
    for (auto& p : m_particles) {
        p.pos  += p.vel * dt;
        p.vel  *= (1.f - 4.f * dt);   // desacelera
        p.life -= dt;
    }
}

void HitEffect::draw(sf::RenderTarget& target) {
    sf::VertexArray va(sf::Quads);
    for (const auto& p : m_particles) {
        if (p.life <= 0.f) continue;
        float alpha = (p.life / m_maxLife);
        sf::Color c = m_color;
        c.a = static_cast<sf::Uint8>(alpha * 220.f);
        float sz = 3.f * alpha + 1.f;

        va.append({{p.pos.x - sz, p.pos.y - sz}, c});
        va.append({{p.pos.x + sz, p.pos.y - sz}, c});
        va.append({{p.pos.x + sz, p.pos.y + sz}, c});
        va.append({{p.pos.x - sz, p.pos.y + sz}, c});
    }
    target.draw(va);
}

bool HitEffect::isFinished() const {
    for (const auto& p : m_particles)
        if (p.life > 0.f) return false;
    return true;
}
