#include "Spectre.hpp"
#include "Map.hpp"
#include <cmath>

static constexpr float PI = 3.14159265f;

Spectre::Spectre(float x, float y) : Enemy(x, y, 75.f, 2, VIEW_RADIUS),
    m_stuckPos({x, y}) {
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

sf::Vector2f Spectre::probeDir(sf::Vector2f dir, float angle, float dt, const Map& map) {
    float c = std::cos(angle), s = std::sin(angle);
    sf::Vector2f d = {dir.x * c - dir.y * s, dir.x * s + dir.y * c};

    // Testa colisão um passo à frente
    sf::Vector2f testPos = position + d * speed * dt * 4.f;
    sprite.setPosition(testPos);
    bool blocked = map.isCollidingWith(sprite.getGlobalBounds());
    sprite.setPosition(position);

    return blocked ? sf::Vector2f{0.f, 0.f} : d;
}

void Spectre::setWaypoints(const std::vector<sf::Vector2f>& wp) {
    m_waypoints = wp;
    m_wpIdx     = 0;
    m_pauseTimer = 0.f;
}

void Spectre::update(float dt, const Map& map, sf::Vector2f playerPos) {
    bool sees = canSeePlayer(playerPos);

    if (sees) {
        m_chasing    = true;
        m_chaseTimer = CHASE_MEMORY;
        float dx = playerPos.x - position.x;
        float dy = playerPos.y - position.y;
        float dist = std::sqrt(dx*dx + dy*dy);
        if (dist > 0.f) m_facingDir = {dx/dist, dy/dist};
    } else if (m_chasing) {
        m_chaseTimer -= dt;
        if (m_chaseTimer <= 0.f) m_chasing = false;
    }

    if (m_chasing) {
        // --- Modo perseguição (com desvio de colisão) ---
        float dx = playerPos.x - position.x;
        float dy = playerPos.y - position.y;
        float dist = std::sqrt(dx*dx + dy*dy);
        if (dist > 0.f) {
            sf::Vector2f desired = {dx/dist, dy/dist};
            float movedSq = (position.x-m_stuckPos.x)*(position.x-m_stuckPos.x)
                          + (position.y-m_stuckPos.y)*(position.y-m_stuckPos.y);
            m_stuckTimer += dt;
            if (m_stuckTimer >= 0.8f) {
                if (movedSq < 4.f && m_escapeTimer <= 0.f) {
                    m_escapeAngle = (std::rand()%2) ? PI/2.f : -PI/2.f;
                    m_escapeTimer = 0.6f;
                }
                m_stuckTimer = 0.f; m_stuckPos = position;
            }
            if (m_escapeTimer > 0.f) m_escapeTimer -= dt;
            float base = (m_escapeTimer > 0.f) ? m_escapeAngle : 0.f;
            static const float PROBES[] = {0.f,PI/6.f,-PI/6.f,PI/3.f,-PI/3.f,PI/2.f,-PI/2.f};
            sf::Vector2f chosen = {};
            for (float offset : PROBES) {
                sf::Vector2f c = probeDir(desired, base+offset, dt, map);
                if (c.x!=0.f || c.y!=0.f) { chosen=c; break; }
            }
            if (chosen.x==0.f && chosen.y==0.f) chosen = desired;
            applyDirection(chosen);
            m_facingDir = chosen;
            position.x += chosen.x * speed * dt;
            sprite.setPosition(position);
            if (map.isCollidingWith(sprite.getGlobalBounds())) {
                position.x -= chosen.x * speed * dt;
                sprite.setPosition(position);
            }
            position.y += chosen.y * speed * dt;
            sprite.setPosition(position);
            if (map.isCollidingWith(sprite.getGlobalBounds())) {
                position.y -= chosen.y * speed * dt;
                sprite.setPosition(position);
            }
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
