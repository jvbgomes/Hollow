#include "Game.hpp"
#include "Shadow.hpp"
#include "Spectre.hpp"
#include "Boss.hpp"
#include <cmath>
#include <cstdlib>

Game::Game()
    : state(GameState::Menu),
      player(100.f, 100.f),
      totalPages(5),
      exitPosition(700.f, 500.f),
      damageCooldown(1.5f),
      damageTimer(1.5f),
      flickerTimer(0.f),
      gameTimer(0.f),
      keyEPressed(false),
      keyQPressed(false),
      keyEnterPressed(false),
      keyUpPressed(false),
      keyDownPressed(false),
      currentMenuState(MenuState::Main),
      mainMenuOption(0),
      characterOption(0),
      selectedSkin(0)
{
    window.create(sf::VideoMode(800, 600), "Hollow");
    window.setFramerateLimit(60);

    audio.playMusic(MusicTrack::Menu);

    map.load("assets/maps/vestibulo.tmx");

    dialogueBox.loadFont("assets/Arial.ttf");
    eventQueue.loadFont("assets/Arial.ttf");
    pageReader.loadFont("assets/Arial.ttf");

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

    itemPromptText.setFont(font);
    itemPromptText.setCharacterSize(20);
    itemPromptText.setFillColor(sf::Color(255, 255, 255));
    itemPromptText.setStyle(sf::Text::Regular);

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
    enemies.push_back(new Spectre(100.f, 200.f));

    npcTexEleanor.loadFromFile("assets/sprites/npcs/eleanor/eleanor.png");
    npcTexThomas .loadFromFile("assets/sprites/npcs/thomas/thomas.png");
    npcTexCrianca.loadFromFile("assets/sprites/npcs/crianca/crianca.png");
    sf::IntRect npcRect(16, 0, 16, 24);

    NPC* eleanor = new NPC("Eleanor", 250.f, 80.f, npcTexEleanor, npcRect);
    eleanor->addOption("Onde estão as páginas do diário?", {
        {"Você",    "Eleanor, sabe onde estão as páginas do diário?",                         true},
        {"Eleanor", "As páginas estão espalhadas pela mansão inteira...",                     false},
        {"Eleanor", "Uma no escritório, outra no jardim de inverno. A última não sei dizer.", false},
    });
    eleanor->addOption("O que aconteceu aqui?", {
        {"Você",    "O que aconteceu com esta mansão? Com você?",                             true},
        {"Eleanor", "Viemos investigar algo que não devíamos ter perturbado.",                false},
        {"Eleanor", "A Entidade despertou. Agora não há como voltar atrás.",                  false},
    });
    eleanor->addOption("Quem é a Entidade?", {
        {"Você",    "A Entidade... você sabe o que é isso?",                                  true},
        {"Eleanor", "Ela sempre esteve aqui. Muito antes de qualquer um de nós.",             false},
        {"Eleanor", "Não tente entendê-la. Apenas fuja enquanto ainda pode.",                 false},
    });
    npcs.push_back(eleanor);

    NPC* thomas = new NPC("Thomas", 70.f, 200.f, npcTexThomas, npcRect);
    thomas->addOption("Onde está a chave para sair?", {
        {"Você",   "Thomas, preciso sair daqui. Onde está a chave?",                          true},
        {"Thomas", "A chave está no porão. Atrás do espelho quebrado.",                       false},
        {"Thomas", "Mas cuidado com o que habita ali. Não é amigável.",                       false},
    });
    thomas->addOption("Tem algum lugar seguro?", {
        {"Você",   "Existe algum lugar seguro nesta mansão?",                                 true},
        {"Thomas", "Seguro? Aqui não existe isso.",                                           false},
        {"Thomas", "Mas o porão tem algo estranho. As sombras hesitam antes de entrar.",      false},
    });
    npcs.push_back(thomas);

    NPC* crianca = new NPC("A Criança", 260.f, 195.f, npcTexCrianca, npcRect);
    crianca->addOption("Quem é você?", {
        {"Você",      "Menino... quem é você? O que está fazendo aqui?",                      true},
        {"A Criança", "Eu sempre estive aqui. Você é que é novidade.",                        false},
        {"A Criança", "Ela não dorme. Quanto mais você corre... mais ela se alimenta.",       false},
    });
    crianca->addOption("Como saio daqui?", {
        {"Você",      "Preciso sair. Pode me ajudar?",                                        true},
        {"A Criança", "A saída não se acha. Ela se revela.",                                  false},
        {"A Criança", "Reúna o que a mansão esconde. Então ela aparecerá para você.",         false},
    });
    npcs.push_back(crianca);

    pageItemTex.loadFromFile("assets/sprites/items_page.png");
    lampItemTex.loadFromFile("assets/sprites/items_lamp.png");
    keyItemTex.loadFromFile("assets/tilesets/PropsV2.png");

    sf::IntRect fullRect(0, 0, 32, 32);
    sf::IntRect keyRect (0, 16, 16, 16);

    items.addItem(ItemType::Page, { 55.f, 120.f}, pageItemTex, fullRect, 999.f,
        "Diário — Primeiro Dia",
        "A porta fechou atrás de mim. Não foi\n"
        "o vento — ouvi o trinco. Alguém fechou.\n\n"
        "Cheguei à Mansão Voss para investigar\n"
        "três desaparecimentos em dois meses.\n"
        "Pesquisadores contratados pelo próprio\n"
        "município para catalogar o acervo.\n"
        "Nenhum voltou.\n\n"
        "Eleanor ainda está aqui. Reconheci o\n"
        "nome da lista, mas não a pessoa. Ela\n"
        "olha através de mim quando falo.\n"
        "Repete a mesma frase em voz baixa:\n"
        "'ela não dorme, ela não dorme'.\n\n"
        "Preciso encontrar uma saída.\n"
        "Antes que o sol desapareça de vez.");

    items.addItem(ItemType::Page, {270.f, 150.f}, pageItemTex, fullRect, 999.f,
        "Nota de Eleanor — Sobre a Entidade",
        "Errei ao chamar de sobrenatural.\n"
        "Sobrenatural implica exceção à regra.\n"
        "Isso aqui é a própria regra.\n\n"
        "Encontrei registros do século XVIII\n"
        "descrevendo 'a presença que habita\n"
        "o subsolo do monte'. Sempre esteve.\n"
        "A mansão foi construída sobre ela.\n\n"
        "O que descobri é perturbador:\n"
        "ela não persegue por instinto.\n"
        "Ela persegue porque você foge.\n"
        "O medo é o alimento. A fuga é o convite.\n\n"
        "Não há como destruí-la.\n"
        "Mas há como sair. A saída existe.\n"
        "Eu só não consigo mais me mover.");

    items.addItem(ItemType::Page, {140.f, 200.f}, pageItemTex, fullRect, 999.f,
        "Registro da Construção — 1891",
        "Ao Sr. Arquiteto Brenner,\n\n"
        "Interrompa as escavações do porão.\n"
        "Os trabalhadores relatam sons durante\n"
        "a noite. Três abandonaram o canteiro.\n"
        "Um deles não foi encontrado.\n\n"
        "Continuamos assim mesmo.\n\n"
        "As sombras vieram primeiro — figuras\n"
        "sem forma nos corredores ao anoitecer.\n"
        "Depois vieram os espectros, mais\n"
        "definidos, com algo parecido com rosto.\n"
        "Por último, Ela.\n\n"
        "Não sabemos o nome correto.\n"
        "Os trabalhadores chamam de A Entidade.\n"
        "O nome parece suficiente.\n\n"
        "— Arq. M. Voss, proprietário");

    items.addItem(ItemType::Page, { 80.f, 160.f}, pageItemTex, fullRect, 999.f,
        "Anotações de Thomas — Sem Data",
        "Observei por semanas.\n\n"
        "As sombras evitam o porão.\n"
        "Há algo no espelho antigo que as\n"
        "repele — talvez o reflexo as confunda,\n"
        "talvez seja outra coisa. Não importa.\n"
        "O fato é: elas não entram.\n\n"
        "Escondi a chave da saída principal\n"
        "atrás do espelho. É o único lugar\n"
        "que posso garantir que estará lá.\n\n"
        "Se você está lendo isso, chegou até\n"
        "aqui. Isso já diz algo sobre você.\n\n"
        "Vá ao porão. Pegue a chave.\n"
        "Use na porta do fundo, lado leste.\n"
        "Não pare para entender.\n"
        "Não olhe para trás.\n"
        "Apenas saia.");

    items.addItem(ItemType::Page, {200.f,  90.f}, pageItemTex, fullRect, 999.f,
        "A Última Mensagem — M.V.",
        "Encontrei as páginas. Falei com todos.\n"
        "Thomas me deu a chave.\n"
        "Tinha tudo que precisava.\n\n"
        "A saída estava a dez passos.\n"
        "Senti o ar frio do lado de fora.\n"
        "Vi a luz lá fora.\n\n"
        "Ouvi algo atrás de mim.\n"
        "Um som que não era vento.\n"
        "Não era madeira rangendo.\n\n"
        "Olhei para trás.\n\n"
        "Ela estava lá.\n"
        "Sempre esteve.\n"
        "Só estava esperando eu perceber.\n\n"
        "Se você encontrar isso —\n"
        "não cometa o mesmo erro.\n\n"
        "— M.V., novembro de 1987");

    items.addItem(ItemType::Lamp, {150.f,  70.f}, lampItemTex, fullRect);
    items.addItem(ItemType::Lamp, {220.f,  70.f}, lampItemTex, fullRect);
    items.addItem(ItemType::Key,  {160.f,  80.f}, keyItemTex,  keyRect);
}

void Game::resetGame() {
    for (Enemy* e : enemies) delete e;
    for (NPC* n : npcs)     delete n;
    enemies.clear();
    npcs.clear();
    projectiles.clear();
    items.clear();
    player = Player(100.f, 100.f);
    // Recarrega textura: sf::Sprite guarda ponteiro que fica inválido após reatribuição
    player.load(selectedSkin == 1 ? "assets/sprites/player/player_f.png"
                                  : "assets/sprites/player/player_m.png");
    damageTimer = 1.5f;
    gameTimer = 0.f;
    hitEffects.clear();
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
    audio.update(dt);

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
                        audio.playMusic(MusicTrack::Explore);
                    } else {
                        currentMenuState = MenuState::CharacterSelect;
                    }
                    keyEnterPressed = true;
                }
            } else if (currentMenuState == MenuState::CharacterSelect) {
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
                    characterOption = 0;
                }
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
                    characterOption = 1;
                }
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) || sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
                    characterOption = 2;
                }
                if ((sf::Keyboard::isKeyPressed(sf::Keyboard::Up) || sf::Keyboard::isKeyPressed(sf::Keyboard::W)) && characterOption == 2) {
                    characterOption = 0;
                }

                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Enter) && !keyEnterPressed) {
                    if (characterOption == 2) {
                        currentMenuState = MenuState::Main;
                    } else {
                        selectedSkin = characterOption;
                        currentMenuState = MenuState::Main;
                    }
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
                audio.playMusic(MusicTrack::Menu);
                keyEnterPressed = true;
            }
            break;
    }
}

void Game::updatePlaying(float dt) {
    // E fecha a página (com prioridade sobre diálogo)
    if (pageReader.isOpen()) {
        audio.updateFootstep(false, false, dt);
        pageReader.handleInput();
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::E) && !keyEPressed) {
            pageReader.close();
            audio.playSfx(SfxId::BookClose, 75.f);
            keyEPressed = true;
        }
        if (!sf::Keyboard::isKeyPressed(sf::Keyboard::E)) keyEPressed = false;
        return;
    }

    if (!dialogueBox.isActive())
        player.update(dt, map, player.getPosition());

    // Passos
    audio.updateFootstep(player.isMoving(), player.isSprinting(), dt);

    // Música dinâmica por contexto
    bool dialogueActive = dialogueBox.isActive();
    if (dialogueActive && !m_dialogueWasActive) {
        audio.playMusic(MusicTrack::Dialogue);
        m_pendingTrack = MusicTrack::Dialogue;
        m_trackTimer   = TRACK_HOLD;
    } else if (!dialogueActive && m_dialogueWasActive) {
        audio.playMusic(MusicTrack::Explore);
        m_pendingTrack = MusicTrack::Explore;
        m_trackTimer   = TRACK_HOLD;
    } else if (!dialogueActive) {
        float nearest = 99999.f;
        bool  bossOn  = false;
        for (Enemy* e : enemies) {
            if (!e->isAlive()) continue;
            float dx = e->getPosition().x - player.getPosition().x;
            float dy = e->getPosition().y - player.getPosition().y;
            float d  = std::sqrt(dx*dx + dy*dy);
            if (d < nearest) nearest = d;
            if (dynamic_cast<Boss*>(e)) bossOn = true;
        }

        MusicTrack wanted;
        if (bossOn)           wanted = MusicTrack::Boss;
        else if (nearest < 90.f)  wanted = MusicTrack::Chase;
        else if (nearest < 180.f) wanted = MusicTrack::EnemyNearby;
        else                      wanted = MusicTrack::Explore;

        // Debounce: só troca após o mesmo contexto ser estável por TRACK_HOLD segundos
        if (wanted != m_pendingTrack) {
            m_pendingTrack = wanted;
            m_trackTimer   = 0.f;
        } else {
            m_trackTimer += dt;
            if (m_trackTimer >= TRACK_HOLD)
                audio.playMusic(m_pendingTrack);
        }
    }
    m_dialogueWasActive = dialogueActive;

    // Growl do Boss: toca quando ele entra no raio, com cooldown
    m_bossGrowlCooldown -= dt;
    bool bossNearby = false;
    for (Enemy* e : enemies) {
        if (!e->isAlive()) continue;
        if (dynamic_cast<Boss*>(e) == nullptr) continue;
        float dx = e->getPosition().x - player.getPosition().x;
        float dy = e->getPosition().y - player.getPosition().y;
        if (std::sqrt(dx*dx + dy*dy) < 200.f) { bossNearby = true; break; }
    }
    if (bossNearby && !m_bossWasNearby && m_bossGrowlCooldown <= 0.f) {
        audio.playSfx(SfxId::BossGrowl, 90.f);
        m_bossGrowlCooldown = 8.f;
    }
    m_bossWasNearby = bossNearby;

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
            audio.playSfx(SfxId::LanternThrow);
            eventQueue.enqueue("Lamparina arremessada  [" + std::to_string(player.getSaltLanterns()) + " restante(s)]");
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

    for (auto& fx : hitEffects) fx.update(dt);
    hitEffects.erase(
        std::remove_if(hitEffects.begin(), hitEffects.end(),
                       [](const HitEffect& fx){ return fx.isFinished(); }),
        hitEffects.end());
}

void Game::checkEnemyPlayerCollision() {
    if (damageTimer < damageCooldown) return;

    for (Enemy* e : enemies) {
        if (!e->isAlive()) continue;
        float dx = e->getPosition().x - player.getPosition().x;
        float dy = e->getPosition().y - player.getPosition().y;
        if (std::sqrt(dx*dx + dy*dy) < 12.f) {
            player.takeDamage();
            damageTimer = 0.f;
            audio.playSfx(SfxId::Damage);
            if (player.getHealth() <= 0) {
                state = GameState::GameOver;
                audio.stopSfx(SfxId::Footstep);
                audio.playMusic(MusicTrack::GameOver);
            } else {
                eventQueue.enqueue("Você foi atingido!  [Vida: " + std::to_string(player.getHealth()) + "/3]");
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
            if (p.getBounds().intersects(e->getHitbox())) {
                e->takeDamage();
                p.deactivate();
                audio.playSfx(SfxId::LanternImpact);

                // Feedback visual — Boss não recebe partículas
                if (!dynamic_cast<Boss*>(e)) {
                    sf::Color col = dynamic_cast<Spectre*>(e)
                        ? sf::Color(100, 200, 255)   // Spectre: azul/ciano
                        : sf::Color(255, 180,  40);  // Shadow: laranja/fogo
                    hitEffects.emplace_back(e->getPosition(), col);
                }

                if (!e->isAlive())
                    eventQueue.enqueue("Inimigo derrotado!");
                break;
            }
        }
    }
}

void Game::checkItemCollection() {
    sf::FloatRect pb = player.getBounds();
    sf::Vector2f  nearPos;
    ItemType      nearType;
    m_itemNearby = items.getNearbyItemInfo(pb, nearPos, nearType);

    if (m_itemNearby) {
        m_itemPromptPos = nearPos;
        switch (nearType) {
            case ItemType::Page: m_itemPromptLabel = "[E] livro";      break;
            case ItemType::Lamp: m_itemPromptLabel = "[E] lamparina";  break;
            case ItemType::Key:  m_itemPromptLabel = "[E] chave";      break;
        }
    }

    if (!m_itemNearby || !sf::Keyboard::isKeyPressed(sf::Keyboard::E) || keyEPressed) return;

    ItemType    collected;
    std::string loreTitle, loreBody;
    if (items.checkAutoCollect(pb, collected, loreTitle, loreBody)) {
        keyEPressed = true;
        switch (collected) {
            case ItemType::Page:
                player.addPage();
                audio.playSfx(SfxId::PageCollect);
                eventQueue.enqueue("Página do diário encontrada  [" +
                    std::to_string(player.getDiaryPages()) + "/" +
                    std::to_string(totalPages) + "]");
                if (!loreBody.empty()) {
                    pageReader.open(loreTitle, loreBody);
                    audio.playSfx(SfxId::BookOpen, 75.f);
                }
                break;
            case ItemType::Lamp:
                player.addLantern();
                audio.playSfx(SfxId::ItemCollect);
                eventQueue.enqueue("Lamparina de sal coletada  [" +
                    std::to_string(player.getSaltLanterns()) + " lamparinas]");
                break;
            case ItemType::Key:
                player.collectKey();
                audio.playSfx(SfxId::ItemCollect);
                eventQueue.enqueue("Chave da mansao encontrada!");
                eventQueue.enqueue("Encontre a saida.");
                break;
        }
    }
}

void Game::checkNPCInteraction() {
    NPC* nearbyNPC = nullptr;
    for (NPC* npc : npcs) {
        if (npc->isPlayerNearby(player.getPosition())) { nearbyNPC = npc; break; }
    }

    m_npcNearby = (nearbyNPC != nullptr && !dialogueBox.isActive());
    if (nearbyNPC)
        m_npcPromptPos = nearbyNPC->getPosition();

    if (!nearbyNPC) {
        if (!sf::Keyboard::isKeyPressed(sf::Keyboard::E)) keyEPressed   = false;
        if (!sf::Keyboard::isKeyPressed(sf::Keyboard::W)) keyUpPressed   = false;
        if (!sf::Keyboard::isKeyPressed(sf::Keyboard::S)) keyDownPressed = false;
        return;
    }

    // Navegação de cursor nas opções
    if (dialogueBox.isShowingOptions()) {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) && !keyUpPressed) {
            dialogueBox.moveCursor(-1);
            audio.playSfx(SfxId::CursorMove, 60.f);
            keyUpPressed = true;
        }
        if (!sf::Keyboard::isKeyPressed(sf::Keyboard::W)) keyUpPressed = false;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) && !keyDownPressed) {
            dialogueBox.moveCursor(+1);
            audio.playSfx(SfxId::CursorMove, 60.f);
            keyDownPressed = true;
        }
        if (!sf::Keyboard::isKeyPressed(sf::Keyboard::S)) keyDownPressed = false;
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::E) && !keyEPressed) {
        if (dialogueBox.isShowingOptions()) {
            int chosen = dialogueBox.confirmOption();
            if (chosen == -1) {
                // "Sair" — já fechou no confirmOption
            } else {
                const DialogueSequence& seq = nearbyNPC->getOptionLines(chosen);
                if (!seq.empty()) {
                    nearbyNPC->markSeen(chosen);
                    dialogueBox.updateOptionSeen(chosen);
                    static const sf::IntRect playerRect(16, 0, 16, 24);
                    dialogueBox.startResponse(seq,
                                              nearbyNPC->getTexture(),
                                              nearbyNPC->getPortraitRect(),
                                              &player.getTexture(),
                                              playerRect);
                }
            }
        } else if (dialogueBox.isActive()) {
            dialogueBox.advanceDialogue();
        } else {
            auto opts = nearbyNPC->getAvailableOptions();
            if (!opts.empty()) {
                audio.playSfx(SfxId::NpcInteract);
                dialogueBox.startOptions(nearbyNPC->getName(),
                                         nearbyNPC->getTexture(),
                                         nearbyNPC->getPortraitRect(),
                                         opts);
            }
        }
        keyEPressed = true;
    }
    if (!sf::Keyboard::isKeyPressed(sf::Keyboard::E)) keyEPressed = false;
}

void Game::checkVictoryCondition() {
    if (player.getDiaryPages() < totalPages) return;
    if (!player.hasKey()) return;

    float dx = player.getPosition().x - exitPosition.x;
    float dy = player.getPosition().y - exitPosition.y;
    if (std::sqrt(dx*dx + dy*dy) < 40.f) {
        state = GameState::Victory;
        audio.playMusic(MusicTrack::Victory);
    }
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

void Game::drawVignette(sf::Color tint) {
    sf::RectangleShape grad(sf::Vector2f(800.f, 600.f));
    grad.setPosition(0.f, 0.f);
    grad.setFillColor(sf::Color(tint.r, tint.g, tint.b, 0)); 
 
    auto edge = [&](float x, float y, float w, float h, int alpha) {
        sf::RectangleShape r(sf::Vector2f(w, h));
        r.setPosition(x, y);
        r.setFillColor(sf::Color(tint.r, tint.g, tint.b, static_cast<sf::Uint8>(alpha)));
        window.draw(r);
    };
 
    edge(0.f,   0.f,   800.f, 40.f,  90); 
    edge(0.f,   560.f, 800.f, 40.f,  90); 
    edge(0.f,   0.f,   40.f,  600.f, 70); 
    edge(760.f, 0.f,   40.f,  600.f, 70);
}


void Game::renderMenu() {
    
    for (const auto& p : dustParticles) {
        sf::CircleShape dot(1.5f);
        dot.setPosition(p.position);
        dot.setFillColor(sf::Color(200, 200, 180, static_cast<sf::Uint8>(p.alpha * 0.4f)));
        window.draw(dot);
    }
 
    if (currentMenuState == MenuState::Main) {
        // --- Glow sutil atrás do título (dá peso sem precisar de shader) ---
        sf::Text titleGlow = titleText;
        titleGlow.setFillColor(sf::Color(60, 70, 90, 60));
        titleGlow.move(0.f, 4.f);
        window.draw(titleGlow);
        window.draw(titleText);
 
        // --- Linha decorativa fina abaixo do título ---
        float lineY = titleText.getPosition().y + titleText.getGlobalBounds().height + 18.f;
        sf::RectangleShape line(sf::Vector2f(180.f, 2.f));
        line.setPosition(400.f - 90.f, lineY);
        line.setFillColor(sf::Color(90, 90, 100, 150));
        window.draw(line);
 
        // Pulsação suave do cursor (efeito "respirando")
        float pulse = 0.6f + 0.4f * std::sin(flickerTimer * 3.f);
        sf::Uint8 selAlpha   = static_cast<sf::Uint8>(200 + 55 * pulse);
        sf::Color selColor (235, 235, 245, selAlpha);
        sf::Color idleColor( 80,  80,  90, 200);
 
        sf::Text optPlay, optSelect;
        optPlay.setFont(font);
        optSelect.setFont(font);
        optPlay.setCharacterSize(24);
        optSelect.setCharacterSize(24);
 
        if (mainMenuOption == 0) {
            optPlay.setString  ("> ADENTRAR A MANSAO <");
            optSelect.setString("  ESCOLHER INVESTIGADOR  ");
            optPlay.setFillColor(selColor);
            optSelect.setFillColor(idleColor);
        } else {
            optPlay.setString  ("  ADENTRAR A MANSAO  ");
            optSelect.setString("> ESCOLHER INVESTIGADOR <");
            optPlay.setFillColor(idleColor);
            optSelect.setFillColor(selColor);
        }
 
        optPlay.setPosition(400.f - optPlay.getGlobalBounds().width / 2.f, 320.f);
        optSelect.setPosition(400.f - optSelect.getGlobalBounds().width / 2.f, 365.f);
 
        window.draw(optPlay);
        window.draw(optSelect);
 
        // Dica de controles, discreta, perto do rodapé
        sf::Text hint;
        hint.setFont(font);
        hint.setCharacterSize(13);
        hint.setFillColor(sf::Color(70, 72, 80));
        hint.setString("W/S move   ENTER confirma");
        hint.setPosition(400.f - hint.getGlobalBounds().width / 2.f, 430.f);
        window.draw(hint);
 
    } else {
        sf::Text smallTitle;
        smallTitle.setFont(font);
        smallTitle.setCharacterSize(28);
        smallTitle.setFillColor(sf::Color(110, 110, 120, 200));
        smallTitle.setStyle(sf::Text::Bold);
        smallTitle.setString("HOLLOW");
        smallTitle.setPosition(400.f - smallTitle.getGlobalBounds().width / 2.f, 70.f);
        window.draw(smallTitle);
 
        sf::Text screenLabel;
        screenLabel.setFont(font);
        screenLabel.setCharacterSize(18);
        screenLabel.setFillColor(sf::Color(150, 150, 160));
        screenLabel.setString("ESCOLHA SEU INVESTIGADOR");
        screenLabel.setPosition(400.f - screenLabel.getGlobalBounds().width / 2.f, 120.f);
        window.draw(screenLabel);
 
        // Linha decorativa abaixo do cabeçalho
        sf::RectangleShape headerLine(sf::Vector2f(180.f, 2.f));
        headerLine.setPosition(400.f - 90.f, 155.f);
        headerLine.setFillColor(sf::Color(90, 90, 100, 150));
        window.draw(headerLine);
 
        const float cardW       = 170.f;
        const float cardH       = 230.f;
        const float cardY       = 195.f;
        const float boyCardX    = 250.f - cardW / 2.f;
        const float girlCardX   = 550.f - cardW / 2.f;
        const float spriteScale = 4.2f;
        const float spriteY     = cardY + 45.f;
        const float nameY       = cardY + cardH - 40.f;
 
        auto drawCard = [&](float cx, bool selected, sf::Color accent) {
            sf::RectangleShape card(sf::Vector2f(cardW, cardH));
            card.setPosition(cx, cardY);
            card.setFillColor(selected ? sf::Color(accent.r, accent.g, accent.b, 22)
                                       : sf::Color(255, 255, 255, 6));
            card.setOutlineThickness(selected ? 2.f : 1.f);
            card.setOutlineColor(selected ? sf::Color(accent.r, accent.g, accent.b, 220)
                                          : sf::Color(70, 70, 78, 140));
            window.draw(card);
 
            // "chão" do personagem
            sf::CircleShape shadow(28.f, 24);
            shadow.setScale(1.f, 0.35f);
            shadow.setFillColor(sf::Color(0, 0, 0, selected ? 90 : 60));
            shadow.setPosition(cx + cardW / 2.f - 28.f, cardY + cardH - 56.f);
            window.draw(shadow);
        };
 
        drawCard(boyCardX,  characterOption == 0, sf::Color(120, 160, 230));
        drawCard(girlCardX, characterOption == 1, sf::Color(230, 140, 170));
 
        sf::Texture texBoy, texGirl;
        if (texBoy.loadFromFile("assets/sprites/player/player_m.png") &&
            texGirl.loadFromFile("assets/sprites/player/player_f.png")) {
 
            sf::Sprite spriteBoy(texBoy);
            spriteBoy.setTextureRect(sf::IntRect(16, 0, 16, 24));
            spriteBoy.setScale(spriteScale, spriteScale);
            spriteBoy.setPosition(250.f - spriteBoy.getGlobalBounds().width / 2.f, spriteY);
 
            sf::Sprite spriteGirl(texGirl);
            spriteGirl.setTextureRect(sf::IntRect(16, 0, 16, 24));
            spriteGirl.setScale(spriteScale, spriteScale);
            spriteGirl.setPosition(550.f - spriteGirl.getGlobalBounds().width / 2.f, spriteY);
 
            // personagem não selecionado fica um pouco apagado
            if (characterOption != 0) spriteBoy.setColor(sf::Color(150, 150, 150, 180));
            if (characterOption != 1) spriteGirl.setColor(sf::Color(150, 150, 150, 180));
 
            window.draw(spriteBoy);
            window.draw(spriteGirl);
        }
 
        sf::Text skinBoyText, skinGirlText;
        skinBoyText.setFont(font);
        skinGirlText.setFont(font);
        skinBoyText.setCharacterSize(19);
        skinGirlText.setCharacterSize(19);
 
        skinBoyText.setString(characterOption == 0 ? "> JOAO <" : "JOAO");
        skinBoyText.setFillColor(characterOption == 0 ? sf::Color(190, 210, 255)
                                                        : sf::Color(95, 95, 100));
 
        skinGirlText.setString(characterOption == 1 ? "> RADLA <" : "RADLA");
        skinGirlText.setFillColor(characterOption == 1 ? sf::Color(255, 195, 215)
                                                          : sf::Color(95, 95, 100));
 
        skinBoyText.setPosition(250.f - skinBoyText.getGlobalBounds().width / 2.f, nameY);
        skinGirlText.setPosition(550.f - skinGirlText.getGlobalBounds().width / 2.f, nameY);
        window.draw(skinBoyText);
        window.draw(skinGirlText);
 
        sf::Text backToMenuText;
        backToMenuText.setFont(font);
        backToMenuText.setCharacterSize(20);
        bool backSelected = (characterOption == 2);
        backToMenuText.setString(backSelected ? "> VOLTAR <" : "VOLTAR");
        backToMenuText.setFillColor(backSelected ? sf::Color::White
                                                  : sf::Color(80, 80, 88));
        backToMenuText.setPosition(400.f - backToMenuText.getGlobalBounds().width / 2.f, 495.f);
        window.draw(backToMenuText);
 
        sf::Text hint;
        hint.setFont(font);
        hint.setCharacterSize(13);
        hint.setFillColor(sf::Color(65, 67, 75));
        hint.setString("A/D escolhe   S desce para Voltar   ENTER confirma");
        hint.setPosition(400.f - hint.getGlobalBounds().width / 2.f, 528.f);
        window.draw(hint);
    }
 
    sf::Text creditsText;
    creditsText.setFont(font);
    creditsText.setCharacterSize(12);
    creditsText.setFillColor(sf::Color(40, 42, 48));
    creditsText.setString("Desenvolvido por Joao V. & Radla O. - LP1 UFRN");
    creditsText.setPosition(400.f - creditsText.getGlobalBounds().width / 2.f, 575.f);
    window.draw(creditsText);
 
    drawVignette(sf::Color::Black);
}
 

void Game::renderPlaying() {
    sf::View camera(player.getPosition(), sf::Vector2f(800.f, 600.f));
    window.setView(camera);

    map.draw(window);
    items.draw(window);

    for (NPC* n : npcs)      n->draw(window);
    for (Enemy* e : enemies)  e->draw(window);
    for (Projectile& p : projectiles) p.draw(window);
    for (HitEffect& fx : hitEffects)  fx.draw(window);

    player.draw(window);

    auto drawWorldPrompt = [&](sf::Vector2f pos, const std::string& label) {
        itemPromptText.setString(sf::String::fromUtf8(label.begin(), label.end()));
        itemPromptText.setScale(0.45f, 0.45f);
        sf::FloatRect tb = itemPromptText.getLocalBounds();
        float cx = pos.x - tb.width * 0.45f / 2.f;
        float cy = pos.y - 20.f;
        itemPromptText.setFillColor(sf::Color(0, 0, 0, 170));
        itemPromptText.setPosition(cx + 0.9f, cy + 0.9f);
        window.draw(itemPromptText);
        itemPromptText.setFillColor(sf::Color(255, 255, 255, 230));
        itemPromptText.setPosition(cx, cy);
        window.draw(itemPromptText);
    };

    if (m_itemNearby && !dialogueBox.isActive() && !pageReader.isOpen())
        drawWorldPrompt(m_itemPromptPos, m_itemPromptLabel);

    if (m_npcNearby && !pageReader.isOpen())
        drawWorldPrompt(m_npcPromptPos, "[E] falar");

    window.setView(window.getDefaultView());

    dialogueBox.draw(window);
    eventQueue.draw(window);
    hud.draw(window);

    pageReader.draw(window);
}

void Game::renderVictory() {
    window.clear(sf::Color(8, 22, 14));
 
    sf::CircleShape glow(260.f);
    glow.setOrigin(260.f, 260.f);
    glow.setPosition(400.f, 260.f);
    glow.setFillColor(sf::Color(40, 90, 60, 40));
    window.draw(glow);
 
    sf::Text title;
    title.setFont(font);
    title.setCharacterSize(40);
    title.setFillColor(sf::Color(150, 230, 170));
    { std::string s = "VOCÊ ESCAPOU";
      title.setString(sf::String::fromUtf8(s.begin(), s.end())); }
    title.setPosition(400.f - title.getGlobalBounds().width / 2.f, 150.f);
    window.draw(title);
 
    sf::RectangleShape line(sf::Vector2f(140.f, 2.f));
    line.setPosition(400.f - 70.f, 210.f);
    line.setFillColor(sf::Color(100, 180, 130, 180));
    window.draw(line);
 
    sf::Text subtitle;
    subtitle.setFont(font);
    subtitle.setCharacterSize(18);
    subtitle.setFillColor(sf::Color(190, 210, 195));
    { std::string s = "A Mansão Voss não te guardou para sempre.";
      subtitle.setString(sf::String::fromUtf8(s.begin(), s.end())); }
    subtitle.setPosition(400.f - subtitle.getGlobalBounds().width / 2.f, 250.f);
    window.draw(subtitle);
 
    // informações da partida
    int   minutes = static_cast<int>(gameTimer) / 60;
    int   seconds = static_cast<int>(gameTimer) % 60;
    char  timeBuf[16];
    std::snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d", minutes, seconds);
 
    sf::Text stats;
    stats.setFont(font);
    stats.setCharacterSize(18);
    stats.setFillColor(sf::Color(170, 200, 180));
    {
        std::string s = "Páginas recuperadas: " + std::to_string(player.getDiaryPages()) +
                         "/" + std::to_string(totalPages) +
                         "      Tempo: " + std::string(timeBuf);
        stats.setString(sf::String::fromUtf8(s.begin(), s.end()));
    }
    stats.setPosition(400.f - stats.getGlobalBounds().width / 2.f, 320.f);
    window.draw(stats);
 
    sf::Text prompt;
    prompt.setFont(font);
    prompt.setCharacterSize(16);
    prompt.setFillColor(sf::Color(120, 150, 130));
    { std::string s = "Pressione ENTER para voltar ao Menu";
      prompt.setString(sf::String::fromUtf8(s.begin(), s.end())); }
    prompt.setPosition(400.f - prompt.getGlobalBounds().width / 2.f, 400.f);
    window.draw(prompt);
 
    drawVignette(sf::Color(0, 20, 5));
}

void Game::renderGameOver() {
    window.clear(sf::Color(20, 4, 4));
 
    sf::CircleShape glow(260.f);
    glow.setOrigin(260.f, 260.f);
    glow.setPosition(400.f, 260.f);
    glow.setFillColor(sf::Color(90, 20, 20, 45));
    window.draw(glow);
 
    sf::Text title;
    title.setFont(font);
    title.setCharacterSize(40);
    title.setFillColor(sf::Color(220, 90, 90));
    { std::string s = "CONSUMIDO PELAS SOMBRAS";
      title.setString(sf::String::fromUtf8(s.begin(), s.end())); }
    title.setPosition(400.f - title.getGlobalBounds().width / 2.f, 150.f);
    window.draw(title);
 
    sf::RectangleShape line(sf::Vector2f(140.f, 2.f));
    line.setPosition(400.f - 70.f, 210.f);
    line.setFillColor(sf::Color(150, 60, 60, 180));
    window.draw(line);
 
    sf::Text subtitle;
    subtitle.setFont(font);
    subtitle.setCharacterSize(18);
    subtitle.setFillColor(sf::Color(200, 170, 170));
    { std::string s = "Ela sempre esteve esperando.";
      subtitle.setString(sf::String::fromUtf8(s.begin(), s.end())); }
    subtitle.setPosition(400.f - subtitle.getGlobalBounds().width / 2.f, 250.f);
    window.draw(subtitle);
 
    sf::Text stats;
    stats.setFont(font);
    stats.setCharacterSize(18);
    stats.setFillColor(sf::Color(200, 150, 150));
    {
        std::string s = "Páginas encontradas antes de cair: " +
                         std::to_string(player.getDiaryPages()) + "/" + std::to_string(totalPages);
        stats.setString(sf::String::fromUtf8(s.begin(), s.end()));
    }
    stats.setPosition(400.f - stats.getGlobalBounds().width / 2.f, 320.f);
    window.draw(stats);
 
    sf::Text prompt;
    prompt.setFont(font);
    prompt.setCharacterSize(16);
    prompt.setFillColor(sf::Color(150, 110, 110));
    { std::string s = "Pressione ENTER para tentar novamente";
      prompt.setString(sf::String::fromUtf8(s.begin(), s.end())); }
    prompt.setPosition(400.f - prompt.getGlobalBounds().width / 2.f, 400.f);
    window.draw(prompt);
 
    drawVignette(sf::Color(15, 0, 0));
}