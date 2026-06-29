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
      keyFPressed(false),
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
    m_lightMap.create(800, 600);

    audio.playMusic(MusicTrack::Menu);

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

    loadRoom("vestibulo", {116.f, 116.f});
}

Game::~Game() {
    for (Enemy* e : enemies) delete e;
    for (NPC* n : npcs)     delete n;
}

void Game::loadRoom(const std::string& room, sf::Vector2f spawnPos) {
    for (Enemy* e : enemies) delete e;
    for (NPC*   n : npcs)    delete n;
    enemies.clear();
    npcs.clear();
    projectiles.clear();

    // Salva quais itens ainda existem na sala atual antes de trocar
    if (!m_currentRoom.empty()) {
        std::set<std::pair<int,int>> present;
        for (const auto& p : items.getPositions())
            present.insert({(int)std::round(p.x), (int)std::round(p.y)});
        m_presentItems[m_currentRoom] = std::move(present);
    }
    items.clear();
    doors.clear();
    transitions.clear();
    hitEffects.clear();

    m_lights.clear();
    m_hallShadowsSpawned = false;
    m_currentRoom = room;
    player.setPosition(spawnPos.x, spawnPos.y);
    map.load("assets/maps/" + room + ".tmx");

    if      (room == "vestibulo")       setupLevel();
    else if (room == "hall_principal")  setupHallPrincipal();
    else if (room == "quarto_crianca")  setupQuartoCrianca();
    else if (room == "biblioteca")      setupBiblioteca();
}

void Game::setupLevel() {
    enemies.push_back(new Spectre(116.f, 216.f));

    npcTexEleanor.loadFromFile("assets/maps/sprites/npcs/eleanor/eleanor.png");
    npcTexThomas .loadFromFile("assets/maps/sprites/npcs/thomas/thomas.png");
    npcTexCrianca.loadFromFile("assets/maps/sprites/npcs/crianca/crianca.png");
    sf::IntRect npcRect(16, 0, 16, 24);

    NPC* eleanor = new NPC("Eleanor", 266.f, 96.f, npcTexEleanor, npcRect);
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

    NPC* thomas = new NPC("Thomas", 86.f, 216.f, npcTexThomas, npcRect);
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


    pageItemTex.loadFromFile("assets/maps/sprites/items_page.png");
    lampItemTex.loadFromFile("assets/maps/sprites/items_lamp.png");
    keyItemTex.loadFromFile("assets/tilesets/PropsV2.png");
    healItemTex.loadFromFile("assets/maps/sprites/items_heal.png");

    sf::IntRect fullRect(0, 0, 32, 32);
    sf::IntRect keyRect (0, 16, 16, 16);

    // Só adiciona itens que ainda não foram coletados/expirados nesta sala
    auto savedIt = m_presentItems.find("vestibulo");
    auto itemOk  = [&](sf::Vector2f pos) -> bool {
        if (savedIt == m_presentItems.end()) return true; // primeira visita
        return savedIt->second.count({(int)std::round(pos.x), (int)std::round(pos.y)}) > 0;
    };

    if (itemOk({ 71.f, 136.f}))
    items.addItem(ItemType::Page, { 71.f, 136.f}, pageItemTex, fullRect, 999.f,
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

    if (itemOk({286.f, 166.f}))
    items.addItem(ItemType::Page, {286.f, 166.f}, pageItemTex, fullRect, 999.f,
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

    if (itemOk({156.f, 216.f}))
    items.addItem(ItemType::Page, {156.f, 216.f}, pageItemTex, fullRect, 999.f,
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

    if (itemOk({ 96.f, 176.f}))
    items.addItem(ItemType::Page, { 96.f, 176.f}, pageItemTex, fullRect, 999.f,
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

    if (itemOk({216.f, 106.f}))
    items.addItem(ItemType::Page, {216.f, 106.f}, pageItemTex, fullRect, 999.f,
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

    if (itemOk({166.f,  86.f})) items.addItem(ItemType::Lamp, {166.f,  86.f}, lampItemTex, fullRect);
    if (itemOk({236.f,  86.f})) items.addItem(ItemType::Lamp, {236.f,  86.f}, lampItemTex, fullRect);
    if (itemOk({176.f,  96.f})) items.addItem(ItemType::Key,  {176.f,  96.f}, keyItemTex,  keyRect);

    // Portas do vestibulo (posicoes apos expansao +1 tile = +16px em x e y)
    // Entrada: tiles (10,3)-(11,3) = x=160-191, y=48-63
    doors.push_back({ {158.f,  44.f,  36.f, 28.f}, Door::Kind::Entrance });
    // Parede esquerda, tiles (1,8)-(1,9): x=16, y=128, h=32
    doors.push_back({ { 16.f, 128.f,  16.f, 32.f}, Door::Kind::Locked });
    // Parede direita, tiles (20,8)-(20,9): x=320, y=128, h=32
    doors.push_back({ {320.f, 128.f,  16.f, 32.f}, Door::Kind::Locked });
    // Passagem sul → hall principal (walk-through, sem E)
    transitions.push_back({ {144.f, 228.f, 80.f, 28.f}, "hall_principal", {208.f, 76.f}, false });

    // Fontes de luz: pos, raioBase, flickerAmt, flickerSpeed, phase
    // Candelabro esq  (tiles 9,2-4): chama no tile (9,2) → pixel (152,40)
    m_lights.push_back({ {152.f, 40.f}, 60.f, 10.f, 2.3f, 0.0f });
    // Candelabro dir  (tiles 12,2-4): chama no tile (12,2) → pixel (200,40)
    m_lights.push_back({ {200.f, 40.f}, 60.f, 10.f, 2.1f, 1.4f });
    // Candelabro mesa (tiles 9-10, 7-8): centro → pixel (160,120)
    m_lights.push_back({ {160.f, 120.f}, 45.f,  7.f, 2.6f, 2.9f });
    // Vela (15,2) → pixel (248,40)
    m_lights.push_back({ {248.f,  40.f}, 40.f,  8.f, 2.8f, 1.7f });
    // Vela (17,14) → pixel (280,232)
    m_lights.push_back({ {280.f, 232.f}, 40.f,  8.f, 2.5f, 3.3f });
}

void Game::setupHallPrincipal() {
    // Transição topo → vestíbulo (player sobe pela escadaria)
    transitions.push_back({ {192.f, 60.f, 32.f, 24.f}, "vestibulo", {176.f, 208.f}, true });

    // Passagem esquerda (cols 0, rows 4-5) → quarto da criança
    transitions.push_back({ {0.f, 62.f, 16.f, 34.f}, "quarto_crianca", {168.f, 120.f}, true });
    // Passagem direita (col 25, rows 4-5) → biblioteca
    transitions.push_back({ {400.f, 62.f, 16.f, 34.f}, "biblioteca", {40.f, 136.f}, true });

    // Fontes de luz do hall
    // (22,1) e (17,3): velas/candelabros individuais no andar superior
    m_lights.push_back({ {360.f,  24.f}, 55.f, 8.f, 2.2f, 0.0f });
    m_lights.push_back({ {280.f,  56.f}, 50.f, 7.f, 2.5f, 1.3f });
    // (7-8, 8-9) e (18-19, 8-9): candelabros 2×2 no andar inferior
    m_lights.push_back({ {128.f, 144.f}, 65.f, 10.f, 2.0f, 2.1f });
    m_lights.push_back({ {304.f, 144.f}, 65.f, 10.f, 2.3f, 0.7f });
}

void Game::setupQuartoCrianca() {
    // Saída direita → hall principal (cols 15-17, rows 5-8 do mapa)
    transitions.push_back({ {255.f, 78.f, 32.f, 68.f}, "hall_principal", {24.f, 76.f}, true });

    // Luzes: (col,row) → pixel centro = col*16, row*16
    m_lights.push_back({ {256.f,  64.f}, 50.f, 8.f, 2.1f, 0.0f });  // (16,4)
    m_lights.push_back({ { 16.f, 112.f}, 45.f, 7.f, 2.4f, 0.8f });  // (1,7)
    m_lights.push_back({ { 32.f, 112.f}, 45.f, 7.f, 2.4f, 1.6f });  // (2,7)

    // NPC: A Criança (centro do quarto)
    npcTexCrianca.loadFromFile("assets/maps/sprites/npcs/crianca/crianca.png");
    sf::IntRect npcRect(16, 0, 16, 24);
    NPC* crianca = new NPC("A Crian\xc3\xa7" "a", 120.f, 120.f, npcTexCrianca, npcRect);
    crianca->addOption("Quem \xc3\xa9 voc\xc3\xaa?", {
        {"Voc\xc3\xaa",      "Menino... quem \xc3\xa9 voc\xc3\xaa? O que est\xc3\xa1 fazendo aqui?",   true},
        {"A Crian\xc3\xa7" "a", "Eu sempre estive aqui. Voc\xc3\xaa \xc3\xa9 que \xc3\xa9 novidade.",  false},
        {"A Crian\xc3\xa7" "a", "Ela n\xc3\xa3o dorme. Quanto mais voc\xc3\xaa corre... mais ela se alimenta.", false},
    });
    crianca->addOption("Como saio daqui?", {
        {"Voc\xc3\xaa",      "Preciso sair. Pode me ajudar?",                                          true},
        {"A Crian\xc3\xa7" "a", "A sa\xc3\xad" "da n\xc3\xa3o se acha. Ela se revela.",                false},
        {"A Crian\xc3\xa7" "a", "Re\xc3\xba" "na o que a mans\xc3\xa3o esconde. Ent\xc3\xa3o ela aparecer\xc3\xa1 para voc\xc3\xaa.", false},
    });
    npcs.push_back(crianca);

    // Item de cura (poção)
    auto savedIt = m_presentItems.find("quarto_crianca");
    auto itemOk  = [&](sf::Vector2f pos) -> bool {
        if (savedIt == m_presentItems.end()) return true;
        return savedIt->second.count({(int)std::round(pos.x), (int)std::round(pos.y)}) > 0;
    };
    sf::IntRect healRect(0, 0, 32, 32);
    if (itemOk({80.f, 160.f})) items.addItem(ItemType::Heal, {80.f, 160.f}, healItemTex, healRect, 999.f);
}

void Game::setupBiblioteca() {
    // Saída esquerda → hall principal (col 0-1, rows 5-9 do mapa)
    transitions.push_back({ {0.f, 78.f, 20.f, 84.f}, "hall_principal", {392.f, 76.f}, true });
}

void Game::resetGame() {
    m_presentItems.clear();
    player = Player(100.f, 100.f);
    player.load(selectedSkin == 1 ? "assets/maps/sprites/player/player_f.png"
                                  : "assets/maps/sprites/player/player_m.png");
    damageTimer = 1.5f;
    gameTimer   = 0.f;

    dialogueBox.close();
    pageReader.close();

    currentMenuState = MenuState::Main;
    mainMenuOption   = 0;
    characterOption  = 0;

    // loadRoom limpa entidades, carrega mapa e popula o cômodo
    loadRoom("vestibulo", {116.f, 116.f});
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
            if (event.key.code == sf::Keyboard::Escape) keyEscPressed = false;
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
               // mainMenuOption: 0 = Adentrar, 1 = Escolher Investigador, 2 = Sair
                if ((sf::Keyboard::isKeyPressed(sf::Keyboard::Up) || sf::Keyboard::isKeyPressed(sf::Keyboard::W))
                    && !keyUpPressed) {
                    mainMenuOption = (mainMenuOption == 0) ? 2 : mainMenuOption - 1;
                    keyUpPressed = true;
                }
                if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Up) && !sf::Keyboard::isKeyPressed(sf::Keyboard::W))
                    keyUpPressed = false;

                if ((sf::Keyboard::isKeyPressed(sf::Keyboard::Down) || sf::Keyboard::isKeyPressed(sf::Keyboard::S))
                    && !keyDownPressed) {
                    mainMenuOption = (mainMenuOption == 2) ? 0 : mainMenuOption + 1;
                    keyDownPressed = true;
                }
                if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Down) && !sf::Keyboard::isKeyPressed(sf::Keyboard::S))
                    keyDownPressed = false;

                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Enter) && !keyEnterPressed) {
                    if (mainMenuOption == 0) {
                        if (selectedSkin == 1)
                            player.load("assets/maps/sprites/player/player_f.png");
                        state = GameState::Playing;
                        audio.playMusic(MusicTrack::Explore);
                    } else if (mainMenuOption == 1) {
                        currentMenuState = MenuState::CharacterSelect;
                    } else {
                        window.close();   // Sair do jogo
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

        case GameState::Paused:
        updatePaused(dt);
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

    //esc para pausar (entrar em um sub-menu)
     if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape) && !keyEscPressed) {
        state = GameState::Paused;
        pauseOption = PauseOption::Continue;
        pauseConfirmRestart = false;
        keyEscPressed = true;
        return;
    }
    if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) keyEscPressed = false;

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
        if (bossOn)            wanted = MusicTrack::Boss;
        else if (nearest < 160.f) wanted = MusicTrack::Chase;
        else if (nearest < 280.f) wanted = MusicTrack::EnemyNearby;
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

    // Spawn dos Shadows ao descer a escadaria (tiles 11-14, row 10)
    if (m_currentRoom == "hall_principal" && !m_hallShadowsSpawned) {
        sf::FloatRect stairExit(176.f, 158.f, 64.f, 20.f);
        if (stairExit.intersects(player.getBounds())) {
            m_hallShadowsSpawned = true;
            enemies.push_back(new Shadow(  8.f, 248.f));
            enemies.push_back(new Shadow(384.f, 240.f));
        }
    }

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
        player.getSaltLanterns(), gameTimer,
        player.getPotions()
    );

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q) && !keyQPressed) {
        player.startAttack();
        keyQPressed = true;
    }
    if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Q)) keyQPressed = false;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::F) && !keyFPressed) {
        keyFPressed = true;
        if (player.usePotion()) {
            audio.playSfx(SfxId::Drink);
            eventQueue.enqueue("Voc\xc3\xaa bebeu a po\xc3\xa7\xc3\xa3o. +1 cora\xc3\xa7\xc3\xa3o.");
        } else if (player.getPotions() <= 0) {
            eventQueue.enqueue("Sem po\xc3\xa7\xc3\xb5" "es.");
        } else {
            eventQueue.enqueue("Vida j\xc3\xa1 est\xc3\xa1 cheia.");
        }
    }
    if (!sf::Keyboard::isKeyPressed(sf::Keyboard::F)) keyFPressed = false;

    if (player.isReadyToThrow()) {
        if (player.useLantern()) {
            projectiles.emplace_back(player.getPosition(), player.getDirection());
            audio.playSfx(SfxId::LanternThrow);
            eventQueue.enqueue("Lamparina arremessada  [" + std::to_string(player.getSaltLanterns()) + " restante(s)]");
        }
    }

    checkDoorInteraction();
    checkTransitions();
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

void Game::updatePaused(float dt) {
    // navegação só roda se não estiver no sub-menu de confirmação
    if (!pauseConfirmRestart) {
        if ((sf::Keyboard::isKeyPressed(sf::Keyboard::Up) || sf::Keyboard::isKeyPressed(sf::Keyboard::W))
            && !keyUpPressed) {
            int idx = static_cast<int>(pauseOption);
            idx = (idx - 1 < 0) ? 2 : idx - 1;
            pauseOption = static_cast<PauseOption>(idx);
            audio.playSfx(SfxId::CursorMove, 60.f);
            keyUpPressed = true;
        }
        if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Up) && !sf::Keyboard::isKeyPressed(sf::Keyboard::W))
            keyUpPressed = false;

        if ((sf::Keyboard::isKeyPressed(sf::Keyboard::Down) || sf::Keyboard::isKeyPressed(sf::Keyboard::S))
            && !keyDownPressed) {
            int idx = static_cast<int>(pauseOption);
            idx = (idx + 1 > 2) ? 0 : idx + 1;
            pauseOption = static_cast<PauseOption>(idx);
            audio.playSfx(SfxId::CursorMove, 60.f);
            keyDownPressed = true;
        }
        if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Down) && !sf::Keyboard::isKeyPressed(sf::Keyboard::S))
            keyDownPressed = false;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape) && !keyEscPressed) {
            state = GameState::Playing;
            keyEscPressed = true;
            return;
        }
        if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) keyEscPressed = false;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Enter) && !keyEnterPressed) {
            switch (pauseOption) {
                case PauseOption::Continue:
                    state = GameState::Playing;
                    break;
                case PauseOption::Restart:
                    pauseConfirmRestart = true;   // abre a confirmação
                    break;
                case PauseOption::Quit:
                    resetGame();  
                    state = GameState::Menu;
                    currentMenuState = MenuState::Main;
                    mainMenuOption = 0;
                    audio.playMusic(MusicTrack::Menu);
                    break;
            }
            keyEnterPressed = true;
        }
        if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Enter)) keyEnterPressed = false;

    } else {

        // ENTER confirma e reinicia, ESC cancela e volta ao menu de pausa
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Enter) && !keyEnterPressed) {
            resetGame();
            state = GameState::Playing;
            audio.playMusic(MusicTrack::Explore);
            pauseConfirmRestart = false;
            keyEnterPressed = true;
        }
        if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Enter)) keyEnterPressed = false;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape) && !keyEscPressed) {
            pauseConfirmRestart = false; // cancela, volta pro menu de pausa normal
            keyEscPressed = true;
        }
        if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) keyEscPressed = false;
    }
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
            case ItemType::Heal: m_itemPromptLabel = "[E] po\xc3\xa7\xc3\xa3o"; break;
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
            case ItemType::Heal:
                player.addPotion();
                audio.playSfx(SfxId::ItemCollect);
                eventQueue.enqueue("Po\xc3\xa7\xc3\xa3o coletada  [F para usar]");
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
        // Diálogo pode ter sido aberto por porta ou outro trigger sem NPC
        if (dialogueBox.isActive()) {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::E) && !keyEPressed) {
                dialogueBox.advanceDialogue();
                keyEPressed = true;
            }
            if (!sf::Keyboard::isKeyPressed(sf::Keyboard::E)) keyEPressed = false;
            return;
        }
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

void Game::checkTransitions() {
    if (dialogueBox.isActive() || pageReader.isOpen()) return;

    sf::FloatRect pb = player.getBounds();
    for (const Transition& t : transitions) {
        if (!t.eRequired) {
            if (t.trigger.intersects(pb)) {
                loadRoom(t.targetRoom, t.spawnPos);
                return;
            }
            continue;
        }

        sf::FloatRect prox = t.trigger;
        prox.left -= 12.f; prox.top -= 12.f;
        prox.width += 24.f; prox.height += 24.f;
        if (!prox.intersects(pb)) continue;

        m_doorNearby = true;
        m_doorPromptPos = { t.trigger.left + t.trigger.width * 0.5f,
                            t.trigger.top - 12.f };

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::E) && !keyEPressed) {
            keyEPressed = true;
            loadRoom(t.targetRoom, t.spawnPos);
        }
        return;
    }
}

void Game::checkDoorInteraction() {
    m_doorNearby = false;

    if (dialogueBox.isActive() || pageReader.isOpen()) return;

    sf::FloatRect pb = player.getBounds();

    for (const Door& d : doors) {
        sf::FloatRect expanded = d.trigger;
        expanded.left   -= 12.f;
        expanded.top    -= 12.f;
        expanded.width  += 24.f;
        expanded.height += 24.f;
        if (!expanded.intersects(pb)) continue;

        m_doorNearby    = true;
        m_doorPromptPos = {
            d.trigger.left + d.trigger.width  / 2.f,
            d.trigger.top  + d.trigger.height / 2.f - 16.f
        };

        if (!sf::Keyboard::isKeyPressed(sf::Keyboard::E) || keyEPressed) return;
        keyEPressed = true;

        static const sf::IntRect playerRect(16, 0, 16, 24);

        switch (d.kind) {
            case Door::Kind::Entrance: {
                std::string line =
                    "A ma\xc3\xa7" "aneta n\xc3\xa3o cede... Algu\xc3\xa9m a prendeu "
                    "de um jeito que n\xc3\xa3o d\xc3\xa1 pra mexer.";
                DialogueSequence seq = {{ "Voc\xc3\xaa", line, true }};
                dialogueBox.startResponse(seq,
                    nullptr, sf::IntRect{},
                    &player.getTexture(), playerRect);
                break;
            }
            case Door::Kind::Locked: {
                eventQueue.enqueue("A porta est\xc3\xa1 emperrada.");
                break;
            }
            case Door::Kind::Exit: {
                if (player.hasKey() && player.getDiaryPages() >= totalPages) {
                    state = GameState::Victory;
                    audio.stopSfx(SfxId::Footstep);
                    audio.playMusic(MusicTrack::Victory);
                } else if (player.hasKey()) {
                    eventQueue.enqueue("A porta cede com a chave, mas ainda h\xc3\xa1 algo aqui.");
                } else {
                    eventQueue.enqueue("A porta est\xc3\xa1 trancada. Precisa de uma chave para sair.");
                }
                break;
            }
        }
        return;
    }
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
        case GameState::Paused:
            renderPlaying();
            renderPaused();
            break;
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
        
        sf::Text titleGlow = titleText;
        titleGlow.setFillColor(sf::Color(60, 70, 90, 60));
        titleGlow.move(0.f, 4.f);
        window.draw(titleGlow);
        window.draw(titleText);

        float lineY = titleText.getPosition().y + titleText.getGlobalBounds().height + 18.f;
        sf::RectangleShape line(sf::Vector2f(180.f, 2.f));
        line.setPosition(400.f - 90.f, lineY);
        line.setFillColor(sf::Color(90, 90, 100, 150));
        window.draw(line);

        float pulse = 0.6f + 0.4f * std::sin(flickerTimer * 3.f);
        sf::Uint8 selAlpha   = static_cast<sf::Uint8>(200 + 55 * pulse);
        sf::Color selColor (235, 235, 245, selAlpha);
        sf::Color idleColor( 80,  80,  90, 200);

        sf::Text optPlay, optSelect, optQuit;
        optPlay.setFont(font);
        optSelect.setFont(font);
        optQuit.setFont(font);
        optPlay.setCharacterSize(24);
        optSelect.setCharacterSize(24);
        optQuit.setCharacterSize(24);

        sf::Color quitSelColor(235, 150, 150, selAlpha);

        optPlay.setString  (mainMenuOption == 0 ? "> ADENTRAR A MANSAO <" : "  ADENTRAR A MANSAO  ");
        optPlay.setFillColor(mainMenuOption == 0 ? selColor : idleColor);

        optSelect.setString(mainMenuOption == 1 ? "> ESCOLHER INVESTIGADOR <" : "  ESCOLHER INVESTIGADOR  ");
        optSelect.setFillColor(mainMenuOption == 1 ? selColor : idleColor);

        optQuit.setString  (mainMenuOption == 2 ? "> SAIR <" : "  SAIR  ");
        optQuit.setFillColor(mainMenuOption == 2 ? quitSelColor : idleColor);

        optPlay.setPosition(400.f - optPlay.getGlobalBounds().width / 2.f, 320.f);
        optSelect.setPosition(400.f - optSelect.getGlobalBounds().width / 2.f, 365.f);
        optQuit.setPosition(400.f - optQuit.getGlobalBounds().width / 2.f, 410.f);

        window.draw(optPlay);
        window.draw(optSelect);
        window.draw(optQuit);

        // Dica de controles
        sf::Text hint;
        hint.setFont(font);
        hint.setCharacterSize(13);
        hint.setFillColor(sf::Color(70, 72, 80));
        hint.setString("W/S move   ENTER confirma");
        hint.setPosition(400.f - hint.getGlobalBounds().width / 2.f, 465.f);
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
 
        // linha do cabeçalho
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
        if (texBoy.loadFromFile("assets/maps/sprites/player/player_m.png") &&
            texGirl.loadFromFile("assets/maps/sprites/player/player_f.png")) {
 
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
    window.clear(sf::Color(0, 0, 0));
    sf::Vector2f camPos(std::round(player.getPosition().x),
                        std::round(player.getPosition().y));
    sf::View camera(camPos, sf::Vector2f(800.f, 600.f));
    window.setView(camera);

    map.draw(window);
    items.draw(window);

    for (NPC* n : npcs)      n->draw(window);
    for (Enemy* e : enemies)  e->draw(window);
    for (Projectile& p : projectiles) p.draw(window);
    for (HitEffect& fx : hitEffects)  fx.draw(window);

    player.draw(window);

    // Balão de interrogação flutuante acima de objetos interativos
    auto drawBalloon = [&](sf::Vector2f worldPos) {
        const float bw = 13.f, bh = 13.f, tailH = 4.f;
        float bx = worldPos.x - bw * 0.5f;
        float by = worldPos.y - bh - tailH - 3.f;

        sf::RectangleShape bg({bw, bh});
        bg.setFillColor(sf::Color(255, 252, 210, 240));
        bg.setOutlineColor(sf::Color(70, 50, 10, 220));
        bg.setOutlineThickness(1.f);
        bg.setPosition(bx, by);
        window.draw(bg);

        float cx = worldPos.x, ty = by + bh;
        sf::VertexArray tail(sf::Triangles, 3);
        tail[0] = {{cx - 3.f, ty},          sf::Color(255, 252, 210, 240)};
        tail[1] = {{cx + 3.f, ty},          sf::Color(255, 252, 210, 240)};
        tail[2] = {{cx,       ty + tailH},  sf::Color(255, 252, 210, 240)};
        window.draw(tail);

        itemPromptText.setString("?");
        itemPromptText.setScale(0.36f, 0.36f);
        sf::FloatRect tb = itemPromptText.getLocalBounds();
        itemPromptText.setFillColor(sf::Color(50, 30, 5, 255));
        itemPromptText.setPosition(
            bx + bw * 0.5f - (tb.left + tb.width  * 0.5f) * 0.36f,
            by + bh * 0.5f - (tb.top  + tb.height * 0.5f) * 0.36f - 0.5f);
        window.draw(itemPromptText);
    };

    if (m_itemNearby && !dialogueBox.isActive() && !pageReader.isOpen())
        drawBalloon(m_itemPromptPos);

    if (m_npcNearby && !pageReader.isOpen())
        drawBalloon(m_npcPromptPos);

    if (m_doorNearby && !dialogueBox.isActive() && !pageReader.isOpen())
        drawBalloon(m_doorPromptPos);

    // --- Iluminação estilo Mad Father ---
    {
        static const float PLAYER_R = 130.f;
        static const int   SEGS     = 48;
        static const float PI2      = 6.28318530f;

        // Lightmap começa escuro; luzes somam cor (aditivo → sobreposição = mais brilhante)
        m_lightMap.clear(sf::Color(12, 10, 16, 255));

        auto drawLight = [&](sf::Vector2f center, float radius, sf::Color inner) {
            sf::VertexArray fan(sf::TriangleFan, SEGS + 2);
            fan[0].position = center;
            fan[0].color    = inner;
            for (int i = 0; i <= SEGS; ++i) {
                float a = i * PI2 / SEGS;
                fan[i + 1].position = { center.x + std::cos(a) * radius,
                                        center.y + std::sin(a) * radius };
                fan[i + 1].color = sf::Color(0, 0, 0, 255);
            }
            m_lightMap.draw(fan, sf::BlendAdd);
        };

        sf::Vector2f camCenter(std::round(player.getPosition().x),
                               std::round(player.getPosition().y));

        // Velas/candelabros: tom quente de chama
        for (const LightSource& light : m_lights) {
            float flicker = light.flickerAmt * (
                std::sin(gameTimer * light.flickerSpeed         + light.phase)        * 0.6f +
                std::sin(gameTimer * light.flickerSpeed * 1.7f + light.phase * 2.1f) * 0.4f);
            float radius = light.baseRadius + flicker;
            sf::Vector2f sp = light.pos - camCenter + sf::Vector2f(400.f, 300.f);
            drawLight(sp, radius, sf::Color(255, 190, 80, 255));
        }

        // Luz do player: levemente fria (lanterna)
        drawLight({400.f, 300.f}, PLAYER_R, sf::Color(210, 215, 255, 255));

        m_lightMap.display();
        window.setView(window.getDefaultView());
        sf::Sprite overlay(m_lightMap.getTexture());
        // Multiply: onde o lightmap é escuro, o mundo escurece proporcionalmente
        static const sf::BlendMode mulBlend(
            sf::BlendMode::DstColor, sf::BlendMode::Zero);
        window.draw(overlay, mulBlend);
    }
    // --- fim da iluminação ---

    dialogueBox.draw(window);
    eventQueue.draw(window);
    hud.draw(window);

    pageReader.draw(window);
}


void Game::renderPaused() {
    window.setView(window.getDefaultView());

    sf::RectangleShape dim(sf::Vector2f(800.f, 600.f));
    dim.setPosition(0.f, 0.f);
    dim.setFillColor(sf::Color(0, 0, 0, 165));
    window.draw(dim);

    sf::Text pauseTitle;
    pauseTitle.setFont(font);
    pauseTitle.setCharacterSize(40);
    pauseTitle.setFillColor(sf::Color(220, 220, 230));
    pauseTitle.setStyle(sf::Text::Bold);
    { std::string s = "PAUSADO";
      pauseTitle.setString(sf::String::fromUtf8(s.begin(), s.end())); }
    pauseTitle.setPosition(260.f - pauseTitle.getGlobalBounds().width / 2.f, 130.f);
    window.draw(pauseTitle);

    sf::RectangleShape line(sf::Vector2f(160.f, 2.f));
    line.setPosition(260.f - 80.f, 185.f);
    line.setFillColor(sf::Color(120, 120, 130, 200));
    window.draw(line);

    auto drawOption = [&](const std::string& label, PauseOption opt, float y) {
        bool selected = (pauseOption == opt) && !pauseConfirmRestart;
        sf::Text t;
        t.setFont(font);
        t.setCharacterSize(24);
        t.setFillColor(selected ? sf::Color(255, 255, 255) : sf::Color(110, 110, 120));
        std::string s = selected ? "> " + label + " <" : label;
        t.setString(sf::String::fromUtf8(s.begin(), s.end()));
        t.setPosition(260.f - t.getGlobalBounds().width / 2.f, y);
        window.draw(t);
    };

    drawOption("CONTINUAR", PauseOption::Continue, 240.f);
    drawOption("REINICIAR", PauseOption::Restart,  290.f);
    drawOption("SAIR",      PauseOption::Quit,      340.f);

    sf::Text hint;
    hint.setFont(font);
    hint.setCharacterSize(13);
    hint.setFillColor(sf::Color(90, 90, 100));
    hint.setString("W/S navega   ENTER confirma   ESC continua");
    hint.setPosition(260.f - hint.getGlobalBounds().width / 2.f, 400.f);
    window.draw(hint);

    sf::RectangleShape statsPanel(sf::Vector2f(260.f, 230.f));
    statsPanel.setPosition(500.f, 150.f);
    statsPanel.setFillColor(sf::Color(255, 255, 255, 10));
    statsPanel.setOutlineThickness(1.f);
    statsPanel.setOutlineColor(sf::Color(255, 255, 255, 50));
    window.draw(statsPanel);

    sf::Text statsTitle;
    statsTitle.setFont(font);
    statsTitle.setCharacterSize(16);
    statsTitle.setFillColor(sf::Color(170, 170, 180));
    { std::string s = "PROGRESSO";
      statsTitle.setString(sf::String::fromUtf8(s.begin(), s.end())); }
    statsTitle.setPosition(520.f, 165.f);
    window.draw(statsTitle);

    int  minutes = static_cast<int>(gameTimer) / 60;
    int  seconds = static_cast<int>(gameTimer) % 60;
    char timeBuf[16];
    std::snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d", minutes, seconds);

    int aliveEnemies = 0;
    for (Enemy* e : enemies) if (e->isAlive()) ++aliveEnemies;

    auto drawStatLine = [&](const std::string& label, const std::string& value, float y) {
        sf::Text lt, vt;
        lt.setFont(font); vt.setFont(font);
        lt.setCharacterSize(15); vt.setCharacterSize(15);
        lt.setFillColor(sf::Color(140, 140, 150));
        vt.setFillColor(sf::Color(225, 225, 230));
        std::string ls = label, vs = value;
        lt.setString(sf::String::fromUtf8(ls.begin(), ls.end()));
        vt.setString(sf::String::fromUtf8(vs.begin(), vs.end()));
        lt.setPosition(520.f, y);
        vt.setPosition(710.f - vt.getGlobalBounds().width, y);
        window.draw(lt);
        window.draw(vt);
    };

    drawStatLine("Tempo decorrido",   std::string(timeBuf),                                   205.f);
    drawStatLine("Páginas do diário", std::to_string(player.getDiaryPages()) + "/" + std::to_string(totalPages), 235.f);
    drawStatLine("Lamparinas de sal", std::to_string(player.getSaltLanterns()),                265.f);
    drawStatLine("Vida",              std::to_string(player.getHealth()) + "/3",               295.f);
    drawStatLine("Chave da mansão",   player.hasKey() ? "Sim" : "Não",                         325.f);
    drawStatLine("Inimigos ativos",   std::to_string(aliveEnemies),                             355.f);

    if (pauseConfirmRestart) {
        sf::RectangleShape confirmBox(sf::Vector2f(440.f, 160.f));
        confirmBox.setPosition(400.f - 220.f, 220.f);
        confirmBox.setFillColor(sf::Color(20, 12, 12, 235));
        confirmBox.setOutlineThickness(1.5f);
        confirmBox.setOutlineColor(sf::Color(200, 90, 90, 200));
        window.draw(confirmBox);

        sf::Text warn;
        warn.setFont(font);
        warn.setCharacterSize(20);
        warn.setFillColor(sf::Color(230, 130, 130));
        { std::string s = "Reiniciar o jogo?";
          warn.setString(sf::String::fromUtf8(s.begin(), s.end())); }
        warn.setPosition(400.f - warn.getGlobalBounds().width / 2.f, 245.f);
        window.draw(warn);

        sf::Text warnSub;
        warnSub.setFont(font);
        warnSub.setCharacterSize(15);
        warnSub.setFillColor(sf::Color(200, 180, 180));
        { std::string s = "Todo o progresso atual sera perdido.";
          warnSub.setString(sf::String::fromUtf8(s.begin(), s.end())); }
        warnSub.setPosition(400.f - warnSub.getGlobalBounds().width / 2.f, 280.f);
        window.draw(warnSub);

        sf::Text warnHint;
        warnHint.setFont(font);
        warnHint.setCharacterSize(14);
        warnHint.setFillColor(sf::Color(170, 150, 150));
        { std::string s = "ENTER confirma   ESC cancela";
          warnHint.setString(sf::String::fromUtf8(s.begin(), s.end())); }
        warnHint.setPosition(400.f - warnHint.getGlobalBounds().width / 2.f, 335.f);
        window.draw(warnHint);
    }
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