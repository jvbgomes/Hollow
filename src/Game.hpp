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
#include "HUD.hpp"

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
    HUD hud;

    std::vector<Enemy*> enemies;
    std::vector<NPC*> npcs;
    std::vector<Projectile> projectiles;

    struct DustParticle {
        sf::Vector2f position;
        float speed;
        float alpha;
        float oscillationSpeed;
    };
    std::vector<DustParticle> dustParticles;

    sf::Texture placeholderTexture;

    sf::Texture npcTexEleanor;
    sf::Texture npcTexThomas;
    sf::Texture npcTexCrianca;

    sf::Texture pageItemTex;
    sf::Texture lampItemTex;
    sf::Texture keyItemTex;

    sf::Font font;
    sf::Text titleText;
    sf::Text subtitleText;
    sf::Text endText;

    int totalPages;
    sf::Vector2f exitPosition;

    float damageCooldown;
    float damageTimer;
    float flickerTimer;
    float gameTimer;

    bool keyEPressed;
    bool keyQPressed;
    bool keyEnterPressed;

    enum class MenuState { Main, CharacterSelect };
    MenuState currentMenuState;
    int mainMenuOption;
    int characterOption;
    int selectedSkin;

    void handleEvents();
    void update(float deltaTime);
    void render();

    void setupLevel();
    void resetGame();

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