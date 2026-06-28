#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include "Player.hpp"
#include "Map.hpp"
#include "NPC.hpp"
#include "ItemList.hpp"
#include "DialogueBox.hpp"
#include "EventQueue.hpp"
#include "Enemy.hpp"
#include "Projectile.hpp"
#include "HUD.hpp"
#include "PageReader.hpp"
#include "AudioManager.hpp"
#include "HitEffect.hpp"

enum class GameState {
    Menu,
    Playing,
    Paused,
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
    HUD          hud;
    PageReader   pageReader;
    AudioManager audio;

    bool  m_dialogueWasActive  = false;
    float m_bossGrowlCooldown  = 0.f;
    bool  m_bossWasNearby      = false;

    std::vector<Enemy*>     enemies;
    std::vector<NPC*>       npcs;
    std::vector<Projectile> projectiles;
    std::vector<HitEffect>  hitEffects;

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
    sf::Text itemPromptText;

    bool         m_itemNearby     = false;
    sf::Vector2f m_itemPromptPos  = {};
    std::string  m_itemPromptLabel;

    bool         m_npcNearby    = false;
    sf::Vector2f m_npcPromptPos = {};

    struct Door {
        enum class Kind { Exit, Entrance, Locked };
        sf::FloatRect trigger;
        Kind kind;
    };
    std::vector<Door> doors;
    bool         m_doorNearby    = false;
    sf::Vector2f m_doorPromptPos = {};

    MusicTrack m_pendingTrack = MusicTrack::None;
    float      m_trackTimer   = 0.f;
    static constexpr float TRACK_HOLD = 1.5f;

    int totalPages;
    sf::Vector2f exitPosition;

    float damageCooldown;
    float damageTimer;
    float flickerTimer;
    float gameTimer;

    bool keyEPressed;
    bool keyQPressed;
    bool keyEnterPressed;
    bool keyUpPressed;
    bool keyDownPressed;

    enum class PauseOption { Continue, Restart, Quit };
    PauseOption pauseOption = PauseOption::Continue;
    bool pauseConfirmRestart = false; 
    bool keyEscPressed     = false;
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
    void checkDoorInteraction();
    void checkVictoryCondition();

    void drawVignette(sf::Color tint);
    void renderMenu();
    void renderPlaying();
    void updatePaused(float dt);
    void renderPaused();
    void renderVictory();
    void renderGameOver();

public:
    Game();
    ~Game();
    void run();
};