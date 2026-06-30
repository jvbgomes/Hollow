#pragma once

#include "Enemy.hpp"
#include <vector>

class Spectre : public Enemy {
public:
    Spectre(float x, float y);

    void update(float deltaTime, const Map& map, sf::Vector2f playerPosition) override;
    void draw(sf::RenderTarget& target) override;

    bool canSeePlayer(sf::Vector2f playerPos) const;

    // Define rota de patrulha; Spectre ignora colisão nesses movimentos (fantasma)
    void setWaypoints(const std::vector<sf::Vector2f>& wp);

private:
    static constexpr float VIEW_RADIUS    = 120.f;
    static constexpr float VIEW_ANGLE_DEG = 70.f;
    static constexpr float CHASE_MEMORY   = 3.f;

    sf::Vector2f m_facingDir  = {0.f, 1.f};
    bool         m_chasing    = false;
    float        m_chaseTimer = 0.f;

    float        m_stuckTimer  = 0.f;
    sf::Vector2f m_stuckPos    = {};
    float        m_escapeTimer = 0.f;
    float        m_escapeAngle = 0.f;

    // Patrulha
    std::vector<sf::Vector2f> m_waypoints;
    int   m_wpIdx      = 0;
    float m_pauseTimer = 0.f;   // pausa breve ao chegar no waypoint

    sf::VertexArray m_fovCone;
    void rebuildFOVCone();

    sf::Vector2f probeDir(sf::Vector2f dir, float angle, float dt, const Map& map);
};
