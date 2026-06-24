#include "Game.hpp"
#include "Shadow.hpp"
#include "Spectre.hpp"
#include "Boss.hpp"
#include <cmath>
#include <cstdlib>

Game::Game()
    : state(GameState::Menu),
      player(100.f, 100.f),
      totalPages(3),
      exitPosition(700.f, 500.f),
      damageCooldown(1.5f),
      damageTimer(1.5f),
      flickerTimer(0.f),
      gameTimer(0.f),
      keyEPressed(false),
      keyQPressed(false),
      keyEnterPressed(false),
      currentMenuState(MenuState::Main),
      mainMenuOption(0),
      characterOption(0),
      selectedSkin(0)
{
    window.create(sf::VideoMode(800, 600), "Hollow");
    window.setFramerateLimit(60);

    map.load("assets/maps/vestibulo.tmx");

    dialogueBox.loadFont("assets/Arial.ttf");
    eventQueue.loadFont("assets/Arial.ttf");
    hud.loadFont("assets/Arial.ttf");

    sf::Image img;
    img.create(32, 32, sf::Color(150, 150, 150));
    placeholderTexture.loadFromImage(img);

    font.loadFromFile("assets/Arial.ttf");

    titleText.setFont(font);
    titleText.setString("HOLLOW");
    titleText.setCharacterSize(72);
    titleText.setFillColor(sf::Color(100, 100, 100, 200));
    titleText.setPosition(400.f - titleText.getGlobalBounds().width / 2.f, 150.f);

    subtitleText.setFont(font);
    subtitleText.setCharacterSize(22);
    subtitleText.setFillColor(sf::Color(160, 160, 160));

    endText.setFont(font);
    endText.setCharacterSize(26);
    endText.setPosition(40.f, 240.f);

    for (int i = 0; i < 40; ++i) {
        DustParticle p;
        p.position = sf::Vector2f(std::rand() % 800, std::rand() % 600);
        p.speed = 10.f + (std::rand() % 20);
        p.alpha = 50.f + (std::rand() % 150);
        p.oscillationSpeed = 1.f + (std::rand() % 3);
        dustParticles.push_back(p);
    }

    setupLevel();
}

Game::~Game() {
    for (Enemy* e : enemies) delete e;
    for (NPC* n : npcs)     delete n;
}

void Game::setupLevel() {
    // Sem inimigos no vestíbulo — área inicial do jogo

    npcTexEleanor.loadFromFile("assets/sprites/npcs/eleanor/eleanor.png");
    npcTexThomas .loadFromFile("assets/sprites/npcs/thomas/thomas.png");
    npcTexCrianca.loadFromFile("assets/sprites/npcs/crianca/crianca.png");
    sf::IntRect npcRect(16, 0, 16, 24);

    NPC* eleanor = new NPC("Eleanor", 250.f, 80.f, npcTexEleanor, npcRect);
    eleanor->addDialogueOption(0, {
        "As paginas do diario...",
        "Procure nos cantos mais escuros da mansao."
    });
    npcs.push_back(eleanor);

    NPC* thomas = new NPC("Thomas", 70.f, 200.f, npcTexThomas, npcRect);
    thomas->addDialogueOption(0, {
        "A chave esta no porao...",
        "Atras do espelho quebrado. Cuidado com o que la habita."
    });
    npcs.push_back(thomas);

    NPC* crianca = new NPC("A Crianca", 260.f, 195.f, npcTexCrianca, npcRect);
    crianca->addDialogueOption(0, {
        "Ela nao dorme...",
        "Quanto mais voce corre, mais rapido ela fica."
    });
    npcs.push_back(crianca);

    pageItemTex.loadFromFile("assets/sprites/items_page.png");
    lampItemTex.loadFromFile("assets/sprites/items_lamp.png");
    keyItemTex.loadFromFile("assets/tilesets/PropsV2.png");

    sf::IntRect fullRect(0, 0, 32, 32);
    sf::IntRect keyRect (0, 16, 16, 16);

    items.addItem(ItemType::Page, { 55.f, 120.f}, pageItemTex, fullRect);
    items.addItem(ItemType::Page, {270.f, 150.f}, pageItemTex, fullRect);
    items.addItem(ItemType::Page, {140.f, 200.f}, pageItemTex, fullRect);
    items.addItem(ItemType::Lamp, {150.f,  70.f}, lampItemTex, fullRect);
    items.addItem(ItemType::Lamp, {220.f,  70.f}, lampItemTex, fullRect);
    items.addItem(ItemType::Key,  {240.f, 200.f}, keyItemTex,  keyRect);
}

void Game::resetGame() {
    for (Enemy* e : enemies) delete e;
    for (NPC* n : npcs)     delete n;
    enemies.clear();
    npcs.clear();
    projectiles.clear();
    items.clear();
    player = Player(100.f, 100.f);
    damageTimer = 1.5f;
    gameTimer = 0.f;
    currentMenuState = MenuState::Main;
    mainMenuOption = 0;
    characterOption = 0;
    setupLevel();
}

void Game::run() {
    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();
        if (dt > 0.1f) dt = 0.1f;

        handleEvents();
        update(dt);
        render();
    }
}

void Game::handleEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed)
            window.close();

        if (event.type == sf::Event::KeyReleased) {
            if (event.key.code == sf::Keyboard::E)     keyEPressed = false;
            if (event.key.code == sf::Keyboard::Q)     keyQPressed = false;
            if (event.key.code == sf::Keyboard::Enter) keyEnterPressed = false;
        }
    }
}

void Game::update(float dt) {
    switch (state) {
        case GameState::Menu:
            flickerTimer += dt;

            for (auto& p : dustParticles) {
                p.position.y -= p.speed * dt;
                p.position.x += std::sin(flickerTimer * p.oscillationSpeed) * 5.f * dt;
                if (p.position.y < -10.f) {
                    p.position.y = 610.f;
                    p.position.x = std::rand() % 800;
                }
            }

            if (currentMenuState == MenuState::Main) {
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) || sf::Keyboard::isKeyPressed(sf::Keyboard::W))
                    mainMenuOption = 0;
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) || sf::Keyboard::isKeyPressed(sf::Keyboard::S))
                    mainMenuOption = 1;
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Enter) && !keyEnterPressed) {
                    if (mainMenuOption == 0) {
                        if (selectedSkin == 1)
                            player.load("assets/sprites/player/player_f.png");
                        state = GameState::Playing;
                    } else {
                        currentMenuState = MenuState::CharacterSelect;
                    }
                    keyEnterPressed = true;
                }
            } else if (currentMenuState == MenuState::CharacterSelect) {
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) || sf::Keyboard::isKeyPressed(sf::Keyboard::W))
                    characterOption = 0;
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) || sf::Keyboard::isKeyPressed(sf::Keyboard::S))
                    characterOption = 1;
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Enter) && !keyEnterPressed) {
                    selectedSkin = characterOption;
                    currentMenuState = MenuState::Main;
                    keyEnterPressed = true;
                }
            }
            break;

        case GameState::Playing:
            updatePlaying(dt);
            break;

        case GameState::Victory:
        case GameState::GameOver:
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Enter) && !keyEnterPressed) {
                resetGame();
                state = GameState::Menu;
                keyEnterPressed = true;
            }
            break;
    }
}

void Game::updatePlaying(float dt) {
    player.update(dt, map, player.getPosition());

    for (Enemy* e : enemies)
        e->update(dt, map, player.getPosition());

    for (auto it = projectiles.begin(); it != projectiles.end(); ) {
        it->update(dt, map);
        if (!it->isActive()) it = projectiles.erase(it);
        else ++it;
    }

    items.update(dt);
    dialogueBox.update(dt);
    eventQueue.update(dt);

    damageTimer += dt;
    gameTimer   += dt;

    hud.update(
        player.getHealth(), 3,
        player.getStamina(), 100.f,
        player.getDiaryPages(), totalPages,
        player.getSaltLanterns(), gameTimer
    );

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q) && !keyQPressed) {
        player.startAttack();
        keyQPressed = true;
    }
    if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Q)) keyQPressed = false;

    if (player.isReadyToThrow()) {
        if (player.useLantern()) {
            projectiles.emplace_back(player.getPosition(), player.getDirection());
            eventQueue.enqueue("Lamparina arremessada  [" + std::to_string(player.getSaltLanterns()) + " restantes]");
        }
    }

    checkNPCInteraction();
    checkEnemyPlayerCollision();
    checkProjectileHits();
    checkItemCollection();
    checkVictoryCondition();

    for (auto it = enemies.begin(); it != enemies.end(); ) {
        if (!(*it)->isAlive()) {
            delete *it;
            it = enemies.erase(it);
        } else {
            ++it;
        }
    }
}

void Game::checkEnemyPlayerCollision() {
    if (damageTimer < damageCooldown) return;

    for (Enemy* e : enemies) {
        if (!e->isAlive()) continue;
        float dx = e->getPosition().x - player.getPosition().x;
        float dy = e->getPosition().y - player.getPosition().y;
        if (std::sqrt(dx*dx + dy*dy) < 30.f) {
            player.takeDamage();
            damageTimer = 0.f;
            if (player.getHealth() <= 0) {
                state = GameState::GameOver;
            } else {
                eventQueue.enqueue("Voce foi atingido!  [Vida: " + std::to_string(player.getHealth()) + "/3]");
            }
            break;
        }
    }
}

void Game::checkProjectileHits() {
    for (Projectile& p : projectiles) {
        if (!p.isActive()) continue;
        for (Enemy* e : enemies) {
            if (!e->isAlive()) continue;
            if (p.getBounds().intersects(e->getBounds())) {
                e->takeDamage();
                p.deactivate();
                if (!e->isAlive())
                    eventQueue.enqueue("Inimigo derrotado!");
                break;
            }
        }
    }
}

void Game::checkItemCollection() {
    ItemType collected;
    while (items.checkAutoCollect(player.getBounds(), collected)) {
        switch (collected) {
            case ItemType::Page:
                player.addPage();
                eventQueue.enqueue("Pagina do diario encontrada  [" +
                    std::to_string(player.getDiaryPages()) + "/" +
                    std::to_string(totalPages) + "]");
                break;
            case ItemType::Lamp:
                player.addLantern();
                eventQueue.enqueue("Lamparina de sal coletada  [" +
                    std::to_string(player.getSaltLanterns()) + " lamparinas]");
                break;
            case ItemType::Key:
                player.collectKey();
                eventQueue.enqueue("Chave da mansao encontrada!  Encontre a saida.");
                break;
        }
    }
}

void Game::checkNPCInteraction() {
    for (NPC* npc : npcs) {
        if (!npc->isPlayerNearby(player.getPosition())) continue;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::E) && !keyEPressed) {
            if (dialogueBox.isActive()) {
                dialogueBox.advanceDialogue();
            } else if (npc->hasDialogue(0)) {
                std::string sentence = npc->getNextSentence(0);
                dialogueBox.startDialogue(npc->getName(), sentence, placeholderTexture, sf::IntRect(0, 0, 32, 32));
            }
            keyEPressed = true;
        }
        break;
    }
    if (!sf::Keyboard::isKeyPressed(sf::Keyboard::E)) keyEPressed = false;
}

void Game::checkVictoryCondition() {
    if (player.getDiaryPages() < totalPages) return;
    if (!player.hasKey()) return;

    float dx = player.getPosition().x - exitPosition.x;
    float dy = player.getPosition().y - exitPosition.y;
    if (std::sqrt(dx*dx + dy*dy) < 40.f)
        state = GameState::Victory;
}

void Game::render() {
    window.clear(sf::Color(12, 14, 18));

    switch (state) {
        case GameState::Menu:     renderMenu();     break;
        case GameState::Playing:  renderPlaying();  break;
        case GameState::Victory:  renderVictory();  break;
        case GameState::GameOver: renderGameOver(); break;
    }

    window.display();
}

void Game::renderMenu() {
    for (const auto& p : dustParticles) {
        sf::CircleShape dot(1.5f);
        dot.setPosition(p.position);
        dot.setFillColor(sf::Color(200, 200, 180, static_cast<sf::Uint8>(p.alpha * 0.4f)));
        window.draw(dot);
    }

    window.draw(titleText);

    if (currentMenuState == MenuState::Main) {
        if (mainMenuOption == 0)
            subtitleText.setString("> ADENTRAR A MANSAO <\n  ESCOLHER INVESTIGADOR  ");
        else
            subtitleText.setString("  ADENTRAR A MANSAO  \n> ESCOLHER INVESTIGADOR <");
    } else {
        if (characterOption == 0)
            subtitleText.setString("> INVESTIGADOR (JOAO) <\n  INVESTIGADORA (RADLA)  ");
        else
            subtitleText.setString("  INVESTIGADOR (JOAO)  \n> INVESTIGADORA (RADLA) <");
    }

    subtitleText.setPosition(400.f - subtitleText.getGlobalBounds().width / 2.f, 320.f);
    window.draw(subtitleText);

    sf::Text creditsText;
    creditsText.setFont(font);
    creditsText.setCharacterSize(12);
    creditsText.setFillColor(sf::Color(45, 48, 55));
    creditsText.setString("Desenvolvido por Joao V. & Radla O. - LP1 UFRN");
    creditsText.setPosition(400.f - creditsText.getGlobalBounds().width / 2.f, 560.f);
    window.draw(creditsText);
}

void Game::renderPlaying() {
    sf::View camera(player.getPosition(), sf::Vector2f(800.f, 600.f));
    window.setView(camera);

    map.draw(window);
    items.draw(window);

    for (NPC* n : npcs)      n->draw(window);
    for (Enemy* e : enemies)  e->draw(window);
    for (Projectile& p : projectiles) p.draw(window);

    player.draw(window);

    window.setView(window.getDefaultView());

    dialogueBox.draw(window);
    eventQueue.draw(window);
    hud.draw(window);
}

void Game::renderVictory() {
    window.clear(sf::Color(10, 30, 10));
    endText.setFillColor(sf::Color::Green);
    endText.setString("Voce escapou da mansao!\nPressione ENTER para voltar ao Menu.");
    endText.setPosition(400.f - endText.getGlobalBounds().width / 2.f, 260.f);
    window.draw(endText);
}

void Game::renderGameOver() {
    window.clear(sf::Color(40, 0, 0));
    endText.setFillColor(sf::Color::Red);
    endText.setString("Voce foi consumido pelas sombras...\nPressione ENTER para tentar novamente.");
    endText.setPosition(400.f - endText.getGlobalBounds().width / 2.f, 260.f);
    window.draw(endText);
}
