#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <map>
#include <set>
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
    Intro,
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
    bool  m_hallShadowsSpawned = false;
    float m_bossGrowlCooldown  = 0.f;
    bool  m_bossWasNearby      = false;

    sf::Font m_cinzel;
    int   m_introPhase     = 1;
    float m_introOverlay   = 1.f;
    bool  m_introFadingOut = false;

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
    sf::Texture healItemTex;

    sf::RenderTexture m_lightMap;

    struct LightSource {
        sf::Vector2f pos;
        float baseRadius;
        float flickerAmt;
        float flickerSpeed;
        float phase;
    };
    std::vector<LightSource> m_lights;

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

    struct Transition {
        sf::FloatRect trigger;
        std::string   targetRoom;
        sf::Vector2f  spawnPos;
        bool          eRequired = false;
    };
    std::vector<Transition> transitions;
    std::string m_currentRoom;
    std::map<std::string, std::set<std::pair<int,int>>> m_presentItems;

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
    bool keyFPressed;
    bool keyEnterPressed;
    bool keyUpPressed;
    bool keyDownPressed;

    enum class PauseOption { Continue, Restart, Quit };
    PauseOption pauseOption = PauseOption::Continue;
    bool pauseConfirmRestart = false;
    bool keyEscPressed     = false;
    int mainMenuOption;
    int characterOption;
    int selectedSkin;

    void handleEvents();
    void update(float deltaTime);
    void render();

    void loadRoom(const std::string& room, sf::Vector2f spawnPos);
    void setupLevel();
    void setupHallPrincipal();
    void setupQuartoCrianca();
    void setupBiblioteca();
    void resetGame();

    void updatePlaying(float deltaTime);
    void checkProjectileHits();
    void checkEnemyPlayerCollision();
    void checkItemCollection();
    void checkNPCInteraction();
    void checkDoorInteraction();
    void checkTransitions();
    void checkVictoryCondition();

    void drawVignette(sf::Color tint);
    void renderMenu();
    void updateIntro(float dt);
    void renderIntro();
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