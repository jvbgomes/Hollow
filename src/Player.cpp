#include "Player.hpp"
#include "Map.hpp"
#include <cmath>

const float Player::FRAME_DUR = 0.125f;

Player::Player(float x, float y) : Entity(x, y),
    m_speed(85.f), m_stamina(100.f), m_maxStamina(100.f), m_sprinting(false),
    m_health(3), m_pages(0), m_lanterns(3), m_hasKey(false),
    m_faceDir(FaceDir::Down), m_animFrame(1), m_animTimer(0.f), m_moving(false),
    m_lastDir(0.f, 1.f),
    m_attackState(AttackState::None), m_attackTimer(0.f), m_readyToThrow(false)
{
    load("assets/sprites/player/player_m.png");
}

void Player::load(const std::string& path) {
    if (!texture.loadFromFile(path)) return;
    sprite.setTexture(texture);
    sprite.setOrigin(FRAME_W / 2.f, FRAME_H / 2.f);
    sprite.setPosition(position);
    applyFrame();
}

void Player::startAttack() {
    if (m_attackState != AttackState::None || m_lanterns <= 0) return;
    m_attackState  = AttackState::Preparing;
    m_attackTimer  = 0.f;
    m_readyToThrow = false;
}

bool Player::isReadyToThrow() {
    if (m_readyToThrow) { m_readyToThrow = false; return true; }
    return false;
}

bool Player::isAttacking() const {
    return m_attackState != AttackState::None;
}

bool Player::isMoving() const   { return m_moving; }
bool Player::isSprinting() const { return m_sprinting; }

void Player::updateAttack(float dt) {
    if (m_attackState == AttackState::None) return;

    m_attackTimer += dt;
    if (m_attackState == AttackState::Preparing) {
        sprite.setColor(sf::Color(255, 220, 120)); // tint laranja: preparando
        if (m_attackTimer >= 0.2f) {
            m_attackState  = AttackState::Throwing;
            m_attackTimer  = 0.f;
            m_readyToThrow = true;
        }
    } else if (m_attackState == AttackState::Throwing) {
        sprite.setColor(sf::Color(255, 160, 60)); // tint mais forte: arremessando
        if (m_attackTimer >= 0.15f) {
            m_attackState = AttackState::None;
            m_attackTimer = 0.f;
            sprite.setColor(sf::Color::White);
        }
    }
}

void Player::handleInput() {
    if (isAttacking()) { velocity = {}; m_moving = false; return; }

    velocity = {};
    float spd = m_speed;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) && m_stamina > 0.f) {
        spd *= 1.6f;
        m_sprinting = true;
    } else {
        m_sprinting = false;
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) { velocity.y = -spd; m_faceDir = FaceDir::Up; }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) { velocity.y =  spd; m_faceDir = FaceDir::Down; }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) { velocity.x = -spd; m_faceDir = FaceDir::Left; }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) { velocity.x =  spd; m_faceDir = FaceDir::Right; }

    m_moving = (velocity.x != 0.f || velocity.y != 0.f);

    if (m_moving) {
        float len = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
        m_lastDir = velocity / len;
    }
}

void Player::updateAnim(float dt) {
    if (m_moving) {
        m_animTimer += dt;
        if (m_animTimer >= FRAME_DUR) {
            m_animTimer = 0.f;
            m_animFrame = (m_animFrame + 1) % 4;
        }
    } else {
        m_animFrame  = 1;
        m_animTimer  = 0.f;
    }
    applyFrame();
}

void Player::applyFrame() {
    sprite.setTextureRect(sf::IntRect(
        m_animFrame * FRAME_W,
        static_cast<int>(m_faceDir) * FRAME_H,
        FRAME_W, FRAME_H
    ));
}

void Player::update(float dt, const Map& map, sf::Vector2f) {
    handleInput();

    if (m_sprinting && m_moving && m_stamina > 0.f) {
        m_stamina -= 30.f * dt;
        if (m_stamina < 0.f) m_stamina = 0.f;
    } else if (!m_sprinting && m_stamina < m_maxStamina) {
        m_stamina += 15.f * dt;
        if (m_stamina > m_maxStamina) m_stamina = m_maxStamina;
    }

    position.x += velocity.x * dt;
    sprite.setPosition(position);
    if (map.isCollidingWith(sprite.getGlobalBounds())) {
        position.x -= velocity.x * dt;
        sprite.setPosition(position);
    }

    position.y += velocity.y * dt;
    sprite.setPosition(position);
    if (map.isCollidingWith(sprite.getGlobalBounds())) {
        position.y -= velocity.y * dt;
        sprite.setPosition(position);
    }

    updateAttack(dt);
    updateAnim(dt);

    if (m_hitTimer > 0.f) {
        m_hitTimer   -= dt;
        m_blinkTimer -= dt;
        if (m_blinkTimer <= 0.f) {
            m_visible    = !m_visible;
            m_blinkTimer = 0.08f;
        }
        if (m_hitTimer <= 0.f) {
            m_visible    = true;
            m_hitTimer   = 0.f;
        }
    }
}

void Player::draw(sf::RenderTarget& target) {
    if (m_visible) target.draw(sprite);
}

void Player::takeDamage() {
    m_health--;
    m_hitTimer   = 1.0f;
    m_blinkTimer = 0.f;
    m_visible    = false;
}

int          Player::getHealth()       const { return m_health; }
int          Player::getDiaryPages()   const { return m_pages; }
int          Player::getSaltLanterns() const { return m_lanterns; }
float        Player::getStamina()      const { return m_stamina; }
sf::Vector2f Player::getDirection()    const { return m_lastDir; }
bool         Player::hasKey()          const { return m_hasKey; }

void Player::addPage()    { m_pages++; }
void Player::addLantern() { m_lanterns++; }
void Player::collectKey() { m_hasKey = true; }

bool Player::useLantern() {
    if (m_lanterns > 0) { m_lanterns--; return true; }
    return false;
}
