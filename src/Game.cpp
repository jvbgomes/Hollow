#include "Game.hpp"
#include "Shadow.hpp"
#include "Spectre.hpp"
#include "Boss.hpp"
#include <cmath>

Game::Game()
    : state(GameState::Menu),
      player(100.f, 100.f),
      totalPages(3),
      exitPosition(700.f, 500.f),
      damageCooldown(1.5f),
      damageTimer(1.5f),
      keyEPressed(false),
      keyQPressed(false),
      keyEnterPressed(false)
{
    window.create(sf::VideoMode(800, 600), "Hollow");
    window.setFramerateLimit(60);

    map.load("assets/test.csv", "assets/tileset_test.png");

    dialogueBox.loadFont("assets/Arial.ttf");
    eventQueue.loadFont("assets/Arial.ttf");

    sf::Image img;
    img.create(32, 32, sf::Color(150, 150, 150));
    placeholderTexture.loadFromImage(img);

    font.loadFromFile("assets/Arial.ttf");

    titleText.setFont(font);
    titleText.setString("HOLLOW");
    titleText.setCharacterSize(72);
    titleText.setFillColor(sf::Color::White);
    titleText.setPosition(260.f, 180.f);

    subtitleText.setFont(font);
    subtitleText.setString("Pressione ENTER para jogar");
    subtitleText.setCharacterSize(22);
    subtitleText.setFillColor(sf::Color(160, 160, 160));
    subtitleText.setPosition(215.f, 310.f);

    endText.setFont(font);
    endText.setCharacterSize(26);
    endText.setPosition(40.f, 240.f);

    setupLevel();
}

Game::~Game() {
    for (Enemy* e : enemies) delete e;
    for (NPC* n : npcs)     delete n;
}

void Game::setupLevel() {
    enemies.push_back(new Shadow(400.f, 300.f));
    enemies.push_back(new Shadow(430.f, 310.f));
    enemies.push_back(new Spectre(600.f, 200.f));
    enemies.push_back(new Boss(1500.f, 1500.f));

    NPC* eleanor = new NPC("Eleanor", 200.f, 150.f, placeholderTexture, sf::IntRect(0, 0, 32, 32));
    eleanor->addDialogueOption(0, {
        "As paginas do diario...",
        "Procure nos cantos mais escuros da mansao."
    });
    npcs.push_back(eleanor);

    NPC* thomas = new NPC("Thomas", 350.f, 450.f, placeholderTexture, sf::IntRect(0, 0, 32, 32));
    thomas->addDialogueOption(0, {
        "A chave esta no porao...",
        "Atras do espelho quebrado. Cuidado com o que la habita."
    });
    npcs.push_back(thomas);

    NPC* crianca = new NPC("A Crianca", 500.f, 350.f, placeholderTexture, sf::IntRect(0, 0, 32, 32));
    crianca->addDialogueOption(0, {
        "Ela nao dorme...",
        "Quanto mais voce corre, mais rapido ela fica."
    });
    npcs.push_back(crianca);

    items.addItem(ItemType::Page, {300.f, 200.f}, placeholderTexture, sf::IntRect(0, 0, 32, 32));
    items.addItem(ItemType::Page, {500.f, 400.f}, placeholderTexture, sf::IntRect(0, 0, 32, 32));
    items.addItem(ItemType::Page, {150.f, 450.f}, placeholderTexture, sf::IntRect(0, 0, 32, 32));
    items.addItem(ItemType::Lamp, {250.f, 300.f}, placeholderTexture, sf::IntRect(0, 0, 32, 32));
    items.addItem(ItemType::Lamp, {450.f, 150.f}, placeholderTexture, sf::IntRect(0, 0, 32, 32));
    items.addItem(ItemType::Key,  {600.f, 500.f}, placeholderTexture, sf::IntRect(0, 0, 32, 32));
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
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Enter) && !keyEnterPressed) {
                state = GameState::Playing;
                keyEnterPressed = true;
            }
            break;
        case GameState::Playing:
            updatePlaying(dt);
            break;
        case GameState::Victory:
        case GameState::GameOver:
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Enter) && !keyEnterPressed) {
                window.close();
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

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q) && !keyQPressed) {
        if (player.useLantern()) {
            projectiles.emplace_back(player.getPosition(), player.getDirection());
            eventQueue.enqueue("Lamparina arremessada  [" + std::to_string(player.getSaltLanterns()) + " restantes]");
        }
        keyQPressed = true;
    }
    if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Q)) keyQPressed = false;

    checkNPCInteraction();
    checkEnemyPlayerCollision();
    checkProjectileHits();
    checkItemCollection();
    checkVictoryCondition();

    // Remove inimigos mortos
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
    window.clear(sf::Color::Black);

    switch (state) {
        case GameState::Menu:     renderMenu();     break;
        case GameState::Playing:  renderPlaying();  break;
        case GameState::Victory:  renderVictory();  break;
        case GameState::GameOver: renderGameOver(); break;
    }

    window.display();
}

void Game::renderMenu() {
    window.draw(titleText);
    window.draw(subtitleText);
}

void Game::renderPlaying() {
    // Camera segue o player
    sf::View camera(player.getPosition(), sf::Vector2f(800.f, 600.f));
    window.setView(camera);

    map.draw(window);
    items.draw(window);

    for (NPC* n : npcs)    n->draw(window);
    for (Enemy* e : enemies) e->draw(window);
    for (Projectile& p : projectiles) p.draw(window);

    player.draw(window);

    // HUD e diálogo ficam fixos na tela (view padrão)
    window.setView(window.getDefaultView());

    dialogueBox.draw(window);
    eventQueue.draw(window);
}

void Game::renderVictory() {
    endText.setFillColor(sf::Color::Green);
    endText.setString("Voce escapou da mansao!\nPressione ENTER para sair.");
    window.draw(endText);
}

void Game::renderGameOver() {
    endText.setFillColor(sf::Color::Red);
    endText.setString("Voce foi consumido pelas sombras...\nPressione ENTER para sair.");
    window.draw(endText);
}
