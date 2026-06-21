#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include "Player.hpp"
#include "Map.hpp"
#include "NPC.hpp"
#include "ItemList.hpp"
#include "DialogueBox.hpp"
#include "EventQueue.hpp"
#include "Enemy.hpp"
#include "Projectile.hpp"

enum class GameState {
    Menu,
    Playing,
    Victory,
    GameOver
};

class Game {
private:
    sf::RenderWindow window;
    sf::Clock clock;
    GameState state;

    Player player;
    Map map;
    ItemList items;
    DialogueBox dialogueBox;
    EventQueue eventQueue;

    std::vector<Enemy*> enemies;
    std::vector<NPC*> npcs;
    std::vector<Projectile> projectiles;

    sf::Texture placeholderTexture;
    sf::Font font;
    sf::Text titleText;
    sf::Text subtitleText;
    sf::Text endText;

    int totalPages;
    sf::Vector2f exitPosition;

    float damageCooldown;
    float damageTimer;

    bool keyEPressed;
    bool keyQPressed;
    bool keyEnterPressed;

    void handleEvents();
    void update(float deltaTime);
    void render();

    void setupLevel();

    void updatePlaying(float deltaTime);
    void checkProjectileHits();
    void checkEnemyPlayerCollision();
    void checkItemCollection();
    void checkNPCInteraction();
    void checkVictoryCondition();

    void renderMenu();
    void renderPlaying();
    void renderVictory();
    void renderGameOver();

public:
    Game();
    ~Game();
    void run();
};
