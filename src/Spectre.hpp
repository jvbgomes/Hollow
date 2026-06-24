#pragma once

#include "Enemy.hpp"

class Spectre : public Enemy {
public:
    Spectre(float x, float y);

    void update(float deltaTime, const Map& map, sf::Vector2f playerPosition) override;
    void draw(sf::RenderTarget& target) override;

    bool canSeePlayer(sf::Vector2f playerPos) const;

private:
    static constexpr float VIEW_RADIUS    = 120.f;
    static constexpr float VIEW_ANGLE_DEG = 70.f;
    static constexpr float CHASE_MEMORY   = 3.f;   // segundos perseguindo após perder de vista

    sf::Vector2f m_facingDir  = {0.f, 1.f};
    bool         m_chasing    = false;
    float        m_chaseTimer = 0.f;

    // steering de desvio de obstáculos
    float        m_stuckTimer  = 0.f;
    sf::Vector2f m_stuckPos    = {};
    float        m_escapeTimer = 0.f;   // tempo restante de fuga ativa
    float        m_escapeAngle = 0.f;   // direção de fuga (offset em rad)

    sf::VertexArray m_fovCone;
    void rebuildFOVCone();

    // retorna dir rotacionada por `angle` rad; {} se colide
    sf::Vector2f probeDir(sf::Vector2f dir, float angle, float dt, const Map& map);
};
