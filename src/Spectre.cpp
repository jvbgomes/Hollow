#include "Spectre.hpp"
#include "Map.hpp"
#include <cmath>

static constexpr float PI = 3.14159265f;

Spectre::Spectre(float x, float y) : Enemy(x, y, 75.f, 2, VIEW_RADIUS) {
    loadTextures("assets/maps/sprites/enemies/spectre/", "spectre");
    float scale = 32.f / 52.f;
    sprite.setScale(scale, scale);
    sprite.setOrigin(26.f, 26.f);
    sprite.setPosition(position);

    m_fovCone.setPrimitiveType(sf::TriangleFan);
    rebuildFOVCone();
}

bool Spectre::canSeePlayer(sf::Vector2f playerPos) const {
    float dx   = playerPos.x - position.x;
    float dy   = playerPos.y - position.y;
    float dist = std::sqrt(dx * dx + dy * dy);

    if (dist > VIEW_RADIUS) return false;

    // Ângulo entre a direção de visão e o vetor até o player
    float angleToPlayer  = std::atan2(dy, dx);
    float angleFacing    = std::atan2(m_facingDir.y, m_facingDir.x);
    float diff           = angleToPlayer - angleFacing;

    // Normaliza para [-PI, PI]
    while (diff >  PI) diff -= 2.f * PI;
    while (diff < -PI) diff += 2.f * PI;

    float halfAngle = (VIEW_ANGLE_DEG / 2.f) * (PI / 180.f);
    return std::abs(diff) < halfAngle;
}

void Spectre::rebuildFOVCone() {
    m_fovCone.clear();

    const int   SEGMENTS   = 20;
    float halfAngle = (VIEW_ANGLE_DEG / 2.f) * (PI / 180.f);
    float facingAngle = std::atan2(m_facingDir.y, m_facingDir.x);

    sf::Color centerColor(200, 80, 255, 120);  // roxo mais brilhante no centro
    sf::Color edgeColor  (200, 80, 255,  18);  // suave na borda

    m_fovCone.append({position, centerColor});

    for (int i = 0; i <= SEGMENTS; ++i) {
        float a = facingAngle - halfAngle + (2.f * halfAngle * i / SEGMENTS);
        sf::Vector2f p = position + sf::Vector2f(std::cos(a), std::sin(a)) * VIEW_RADIUS;
        m_fovCone.append({p, edgeColor});
    }
}


void Spectre::setWaypoints(const std::vector<sf::Vector2f>& wp) {
    m_waypoints = wp;
    m_wpIdx     = 0;
    m_pauseTimer = 0.f;
}

void Spectre::update(float dt, const Map& map, sf::Vector2f playerPos) {
    float dx0 = playerPos.x - position.x;
    float dy0 = playerPos.y - position.y;
    float dist0 = std::sqrt(dx0*dx0 + dy0*dy0);

    // Detecta player via FOV ou por proximidade imediata (raio 80px — sem linha de visão necessária)
    bool sees = canSeePlayer(playerPos) || dist0 < 80.f;

    if (sees) {
        m_chasing    = true;
        m_chaseTimer = CHASE_MEMORY;
        if (dist0 > 0.f) m_facingDir = {dx0/dist0, dy0/dist0};
    } else if (m_chasing) {
        m_chaseTimer -= dt;
        if (m_chaseTimer <= 0.f) m_chasing = false;
    }

    if (m_chasing) {
        // --- Modo perseguição: linha reta, ignora móveis (fantasma passa por tudo) ---
        if (dist0 > 0.f) {
            sf::Vector2f dir = {dx0/dist0, dy0/dist0};
            applyDirection(dir);
            m_facingDir = dir;
            // Move sem checagem de colisão — Spectre é fantasma e atravessa mobília
            position.x += dir.x * speed * dt;
            position.y += dir.y * speed * dt;
            sprite.setPosition(position);
        }
    } else if (!m_waypoints.empty()) {
        // --- Modo patrulha: ignora colisão (fantasma atravessa móveis) ---
        if (m_pauseTimer > 0.f) {
            m_pauseTimer -= dt;
        } else {
            sf::Vector2f target = m_waypoints[m_wpIdx];
            float dx = target.x - position.x;
            float dy = target.y - position.y;
            float dist = std::sqrt(dx*dx + dy*dy);
            if (dist < 10.f) {
                // chegou — avança para o próximo waypoint e faz pausa curta
                m_wpIdx = (m_wpIdx + 1) % (int)m_waypoints.size();
                m_pauseTimer = 0.6f + (std::rand()%60)*0.01f; // 0.6-1.2s
            } else {
                float patrolSpeed = speed * 0.55f; // mais devagar na patrulha
                sf::Vector2f dir = {dx/dist, dy/dist};
                applyDirection(dir);
                m_facingDir = dir;
                // move sem checar colisão (atravessa mobília como fantasma)
                position.x += dir.x * patrolSpeed * dt;
                position.y += dir.y * patrolSpeed * dt;
                sprite.setPosition(position);
            }
        }
    }

    rebuildFOVCone();
}

void Spectre::draw(sf::RenderTarget& target) {
    target.draw(m_fovCone);
    Enemy::draw(target);
}
