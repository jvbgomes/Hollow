#pragma once
#include "Entity.hpp"

class Player : public Entity {
public:
    Player(float x, float y);

    void load(const std::string& spritePath);

    void update(float dt, const Map& map, sf::Vector2f playerPos) override;
    void draw(sf::RenderTarget& target) override;

    void takeDamage();

    int          getHealth()       const;
    int          getDiaryPages()   const;
    int          getSaltLanterns() const;
    float        getStamina()      const;
    sf::Vector2f getDirection()    const;
    bool         hasKey()          const;
    bool         isReadyToThrow();
    bool         isAttacking()    const;
    bool         isMoving()       const;
    bool         isSprinting()    const;

    void startAttack();
    void addPage();
    void addLantern();
    void collectKey();
    bool useLantern();
    void heal(int amount = 1);
    void addPotion();
    int  getPotions() const;
    bool usePotion();

private:
    static const int   FRAME_W   = 16;
    static const int   FRAME_H   = 24;
    static const float FRAME_DUR;

    enum class FaceDir { Down = 0, Right = 1, Up = 2, Left = 3 };

    float   m_speed;
    float   m_stamina;
    float   m_maxStamina;
    bool    m_sprinting;
    int     m_health;
    int     m_pages;
    int     m_lanterns;
    bool    m_hasKey;
    int     m_potions;

    FaceDir m_faceDir;
    int     m_animFrame;
    float   m_animTimer;
    bool    m_moving;

    sf::Vector2f m_lastDir;

    enum class AttackState { None, Preparing, Throwing };
    AttackState m_attackState;
    float       m_attackTimer;
    bool        m_readyToThrow;

    float m_hitTimer   = 0.f;
    float m_blinkTimer = 0.f;
    bool  m_visible    = true;

    void handleInput();
    void updateAnim(float dt);
    void applyFrame();
    void updateAttack(float dt);
};
