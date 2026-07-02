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
      mainMenuOption(0),
      characterOption(0),
      selectedSkin(0)
{
    sf::ContextSettings ctx;
    ctx.antialiasingLevel = 8;
    window.create(sf::VideoMode(800, 600), "Hollow", sf::Style::Default, ctx);
    window.setFramerateLimit(60);
    m_lightMap.create(800, 600);

    audio.playMusic(MusicTrack::Menu);

    dialogueBox.loadFont("assets/fonts/Arial.ttf");
    eventQueue.loadFont("assets/fonts/Arial.ttf");
    pageReader.loadFont("assets/fonts/Arial.ttf");

    sf::Image img;
    img.create(32, 32, sf::Color(150, 150, 150));
    placeholderTexture.loadFromImage(img);

    font.loadFromFile("assets/fonts/Arial.ttf");
    m_cinzel.loadFromFile("assets/fonts/Cinzel.ttf");

    titleText.setFont(m_cinzel);
    titleText.setString("HOLLOW");
    titleText.setCharacterSize(90);
    titleText.setLetterSpacing(3.f);
    titleText.setFillColor(sf::Color(120, 122, 132, 210));
    titleText.setPosition(400.f - titleText.getGlobalBounds().width / 2.f, 148.f);

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
    else if (room == "porao")           setupPorao();
    else if (room == "porao_fundo")     setupPoraoFundo();
    else if (room == "deposito")        setupDeposito();
    else if (room == "corredor_saida")  setupCorredorSaida();
    else if (room == "sala_estar")      setupSalaEstar();
    else if (room == "area_externa")    setupAreaExterna();
    else if (room == "cozinha")         setupCozinha();
}

void Game::setupLevel() {
    npcTexCrianca.loadFromFile("assets/maps/sprites/npcs/crianca/crianca.png");


    pageItemTex.loadFromFile("assets/maps/sprites/items_book.png");
    lampItemTex.loadFromFile("assets/maps/sprites/items_candle.png");
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

    // Vestíbulo: apenas lamparinas — páginas e chave redistribuídas pelos cômodos
    if (itemOk({166.f,  86.f})) items.addItem(ItemType::Lamp, {166.f,  86.f}, lampItemTex, fullRect);
    if (itemOk({236.f,  86.f})) items.addItem(ItemType::Lamp, {236.f,  86.f}, lampItemTex, fullRect);

    // Portas do vestibulo — posições centradas nos tiles indicados pelo usuário
    // Entrada mansão: tile (11,3) → centro pixel (184,56); cloud = center - 16px vertical
    doors.push_back({ {168.f, 40.f, 32.f, 32.f}, Door::Kind::Entrance });
    // Porta emperrada esq: tile (1,10) → centro (24,168)
    doors.push_back({ {  8.f, 152.f, 32.f, 32.f}, Door::Kind::Locked });
    // Porta emperrada dir: tile (22,10) → centro (360,168)
    doors.push_back({ {344.f, 152.f, 32.f, 32.f}, Door::Kind::Locked });
    // Passagem sul → hall: cloud próxima ao tile (12,17) → trigger.top=272, cloud y=260
    transitions.push_back({ {160.f, 272.f, 64.f, 32.f}, "hall_principal", {208.f, 76.f}, true, {200.f, 296.f} });

    // Fontes de luz: pos, raioBase, flickerAmt, flickerSpeed, phase
    // Candelabro esq  (tiles 9,2-4): chama no tile (9,2) → pixel (152,40)
    m_lights.push_back({ {152.f, 40.f}, 60.f, 10.f, 2.3f, 0.0f });
    // Candelabro dir  (tiles 12,2-4): chama no tile (12,2) → pixel (200,40)
    m_lights.push_back({ {200.f, 40.f}, 60.f, 10.f, 2.1f, 1.4f });
    // Candelabro mesa (tiles 9-10, 7-8): centro → pixel (160,120)
    m_lights.push_back({ {160.f, 120.f}, 45.f,  7.f, 2.6f, 2.9f });
    // Vela (15,2) → pixel (248,40)
    m_lights.push_back({ {248.f,  40.f}, 40.f,  8.f, 2.8f, 1.7f });
    // Canto esquerdo  (1,8)  → pixel (24,136)
    m_lights.push_back({ { 24.f, 136.f}, 50.f,  9.f, 2.4f, 0.5f });
    // Canto direito   (22,8) → pixel (360,136)
    m_lights.push_back({ {360.f, 136.f}, 50.f,  9.f, 2.2f, 2.1f });
    // Passagem sul esq (14,17) → pixel (232,280)
    m_lights.push_back({ {232.f, 280.f}, 48.f,  8.f, 2.6f, 1.0f });
    // Passagem sul dir (15,17) → pixel (248,280)
    m_lights.push_back({ {248.f, 280.f}, 48.f,  8.f, 2.3f, 3.0f });
    // Canto inferior esq (3,14) → pixel (56,232)
    m_lights.push_back({ { 56.f, 232.f}, 48.f,  8.f, 2.5f, 1.8f });
}

void Game::setupHallPrincipal() {
    // Transição topo → vestíbulo (player sobe pela escadaria)
    transitions.push_back({ {176.f, 48.f, 64.f, 32.f}, "vestibulo", {192.f, 272.f}, true });

    // Passagem esquerda (cols 0, rows 4-5) → quarto da criança
    transitions.push_back({ {0.f, 62.f, 16.f, 34.f}, "quarto_crianca", {168.f, 120.f}, true });
    // Passagem direita (col 25, rows 4-5) → biblioteca
    transitions.push_back({ {400.f, 62.f, 16.f, 34.f}, "biblioteca", {40.f, 200.f}, true });
    // Entrada inferior esquerda → porão (Thomas)
    transitions.push_back({ {0.f, 200.f, 32.f, 32.f}, "porao", {170.f, 100.f}, true, {0.f, 240.f} }); //192 e 48
    // Passagem inferior direita → sala de estar (cols 17-21, rows 16-17)
    transitions.push_back({ {272.f, 256.f, 80.f, 32.f}, "sala_estar", {225.f, 80.f}, true, {400.f, 240.f} });

    // Fontes de luz do hall
    // (22,1) e (17,3): velas/candelabros individuais no andar superior
    m_lights.push_back({ {360.f,  24.f}, 55.f, 8.f, 2.2f, 0.0f });
    m_lights.push_back({ {280.f,  56.f}, 50.f, 7.f, 2.5f, 1.3f });
    // (7-8, 8-9) e (18-19, 8-9): candelabros 2×2 no andar inferior
    m_lights.push_back({ {128.f, 144.f}, 65.f, 10.f, 2.0f, 2.1f });
    m_lights.push_back({ {304.f, 144.f}, 65.f, 10.f, 2.3f, 0.7f });
}

void Game::setupQuartoCrianca() {
    // Saída direita → hall principal (col 17, rows 6-9 = y=96-160)
    transitions.push_back({ {256.f, 96.f, 32.f, 64.f}, "hall_principal", {24.f, 76.f}, true, {264.f, 120.f} }); 

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
    // Saída esquerda → hall principal
    transitions.push_back({ {0.f, 158.f, 20.f, 82.f}, "hall_principal", {392.f, 76.f}, true });

    // Eleanor — próxima ao spawn do player (spawn={40,200}), lado direito da entrada
    npcTexEleanor.loadFromFile("assets/maps/sprites/npcs/eleanor/eleanor.png");
    sf::IntRect npcRect(16, 0, 16, 24);
    NPC* eleanor = new NPC("Eleanor", 80.f, 200.f, npcTexEleanor, npcRect);
    eleanor->setColor(sf::Color(180, 180, 180, 200));
    eleanor->addOption("Onde est\xc3\xa3o as p\xc3\xa1ginas do di\xc3\xa1rio?", {
        {"Voc\xc3\xaa",    "Eleanor, sabe onde est\xc3\xa3o as p\xc3\xa1ginas do di\xc3\xa1rio?",                                      true},
        {"Eleanor", "Cinco p\xc3\xa1ginas... o que sobrou dos nossos \xc3\xbaltimos dias aqui.",                          false},
        {"Eleanor", "Uma est\xc3\xa1 aqui na biblioteca. As criaturas a guardam \xe2\x80\x94 tenha muito cuidado.",       false},
        {"Eleanor", "H\xc3\xa1 outra l\xc3\xa1 fora, na \xc3\xa1rea al\xc3\xa9m da sala de estar. E uma na cozinha.",    false},
        {"Eleanor", "As duas restantes est\xc3\xa3o no por\xc3\xa3o. Thomas sabe onde ficam.",                            false},
    });
    eleanor->addOption("O que aconteceu aqui?", {
        {"Voc\xc3\xaa",    "O que aconteceu com esta mans\xc3\xa3o? Com voc\xc3\xaa?",                             true},
        {"Eleanor", "Viemos investigar algo que n\xc3\xa3o dev\xc3" "\xad" "amos ter perturbado.",         false},
        {"Eleanor", "A Entidade despertou. Agora n\xc3\xa3o h\xc3\xa1 como voltar atr\xc3\xa1s.",                  false},
    });
    eleanor->addOption("Quem \xc3\xa9 a Entidade?", {
        {"Voc\xc3\xaa",    "A Entidade... voc\xc3\xaa sabe o que \xc3\xa9 isso?",                                  true},
        {"Eleanor", "Ela sempre esteve aqui. Muito antes de qualquer um de n\xc3\xb3s.",             false},
        {"Eleanor", "N\xc3\xa3o tente entend\xc3\xaa-la. Apenas fuja enquanto ainda pode.",                 false},
    });
    npcs.push_back(eleanor);

    // Spectres com patrulha — percorrem a biblioteca como guardiões
    // Biblioteca: 52×21 tiles = 832×336 px, spawn player em (40,200)
    {
        auto* s1 = new Spectre(200.f, 100.f);
        s1->setWaypoints({
            {200.f, 80.f}, {380.f, 80.f}, {380.f, 200.f},
            {200.f, 200.f}, {200.f, 280.f}, {80.f, 280.f}, {80.f, 140.f}
        });
        enemies.push_back(s1);
    }
    {
        auto* s2 = new Spectre(580.f, 160.f);
        s2->setWaypoints({
            {500.f, 80.f}, {700.f, 80.f}, {780.f, 180.f},
            {700.f, 290.f}, {500.f, 290.f}, {380.f, 200.f}, {420.f, 100.f}
        });
        enemies.push_back(s2);
    }
    {
        auto* s3 = new Spectre(680.f, 260.f);
        s3->setWaypoints({
            {680.f, 100.f}, {780.f, 100.f}, {800.f, 200.f},
            {780.f, 300.f}, {600.f, 300.f}, {500.f, 180.f}, {600.f, 100.f}
        });
        enemies.push_back(s3);
    }

    // Página do diário
    if (pageItemTex.getSize().x == 0)
        pageItemTex.loadFromFile("assets/maps/sprites/items_book.png");
    auto savedIt = m_presentItems.find("biblioteca");
    auto itemOk  = [&](sf::Vector2f pos) -> bool {
        if (savedIt == m_presentItems.end()) return true;
        return savedIt->second.count({(int)std::round(pos.x), (int)std::round(pos.y)}) > 0;
    };
    sf::IntRect pageRect(0, 0, 32, 32);
    if (itemOk({380.f, 120.f}))
    items.addItem(ItemType::Page, {380.f, 120.f}, pageItemTex, pageRect, 999.f,
        "Di\xc3\xa1rio \xe2\x80\x94 Primeiro Dia",
        "A porta fechou atr\xc3\xa1s de mim. N\xc3\xa3o foi\n"
        "o vento \xe2\x80\x94 ouvi o trinco. Algu\xc3\xa9m fechou.\n\n"
        "Cheguei \xc3\xa0 Mans\xc3\xa3o Voss para investigar\n"
        "o desaparecimento da fam\xc3\xad" "lia. Eleanor,\n"
        "Thomas e a crian\xc3\xa7" "a \xe2\x80\x94 sumidos sem rastro.\n\n"
        "Eleanor ainda est\xc3\xa1 aqui. Reconheci o\n"
        "nome da lista, mas n\xc3\xa3o a pessoa. Ela\n"
        "olha atrav\xc3\xa9s de mim quando falo.\n"
        "Repete a mesma frase em voz baixa:\n"
        "'ela n\xc3\xa3o dorme, ela n\xc3\xa3o dorme'.\n\n"
        "Preciso encontrar uma sa\xc3\xad" "da.\n"
        "Antes que o sol desapare\xc3\xa7" "a de vez.");

    // Iluminação — coordenadas fornecidas pelo usuário
    // Pares de tiles (tile*16+8 = centro do pixel)
    m_lights.push_back({ {432.f,  56.f}, 70.f, 10.f, 2.2f, 0.0f }); // (26,3)+(27,3)
    m_lights.push_back({ {432.f, 264.f}, 70.f, 10.f, 2.0f, 1.2f }); // (26,16)+(27,16)
    m_lights.push_back({ {808.f, 120.f}, 60.f,  8.f, 2.4f, 0.5f }); // (50,7)
    m_lights.push_back({ {808.f, 232.f}, 60.f,  8.f, 2.1f, 2.0f }); // (50,14)
    m_lights.push_back({ {128.f,  56.f}, 65.f,  9.f, 2.3f, 0.8f }); // (7,3)+(8,3)
    m_lights.push_back({ {144.f, 248.f}, 65.f,  9.f, 2.5f, 1.7f }); // (8,15)+(9,15)
    // Lareira (16-17, 5-6) — chama mais quente e brilhante
    m_lights.push_back({ {272.f,  96.f}, 90.f, 18.f, 1.8f, 3.0f });
}

void Game::setupSalaEstar() {
    // Topo → hall principal (cols 13-17, row 0 → pixel x=208-272, y=0)
    transitions.push_back({ {208.f, 0.f, 64.f, 32.f}, "hall_principal", {400.f, 240.f}, true });
    // Baixo → área externa (cols 5-10, row 17 → pixel x=80-160, y=272)
    transitions.push_back({ {80.f, 256.f, 80.f, 32.f}, "area_externa", {20.f, 225.f}, true });
    // Direita → cozinha (col 25, rows 5-9 → pixel x=400, y=80-144)
    transitions.push_back({ {400.f, 80.f, 16.f, 64.f}, "cozinha", {20.f, 200.f}, true });

    m_lights.push_back({ {208.f,  48.f}, 55.f, 9.f, 2.1f, 0.0f });
    m_lights.push_back({ {320.f, 144.f}, 60.f, 8.f, 2.4f, 1.2f });
    m_lights.push_back({ {112.f, 192.f}, 50.f, 7.f, 2.6f, 2.5f });
}

void Game::setupAreaExterna() {
    // Topo → sala de estar (cols 8-13, row 0 → pixel x=128-208, y=0)
    transitions.push_back({ {128.f, 0.f, 80.f, 32.f}, "sala_estar", {120.f, 240.f}, true });

    // Página 2: Anotações de Eleanor (placeholder, ajustar conforme o mapa)
    if (pageItemTex.getSize().x == 0)
        pageItemTex.loadFromFile("assets/maps/sprites/items_book.png");
    sf::IntRect pr(0, 0, 32, 32);
    auto savedIt = m_presentItems.find("area_externa");
    auto itemOk  = [&](sf::Vector2f pos) -> bool {
        if (savedIt == m_presentItems.end()) return true;
        return savedIt->second.count({(int)std::round(pos.x), (int)std::round(pos.y)}) > 0;
    };
    if (itemOk({320.f, 240.f})) {
        items.addItem(ItemType::Page, {320.f, 240.f}, pageItemTex, pr, 999.f,
            "Anota\xc3\xa7\xc3\xb5" "es de Eleanor \xe2\x80\x94 Dia Tr\xc3\xaas",
            "Encontrei rastros no quintal.\n\n"
            "P\xc3\xa9" "gadas no lodo, saindo da\n"
            "porta dos fundos at\xc3\xa9 o centro\n"
            "do jardim. E depois nada.\n\n"
            "Como se a pessoa tivesse\n"
            "simplesmente... parado.\n"
            "No meio do espa\xc3\xa7o aberto.\n\n"
            "Voltei para dentro assim que\n"
            "o vento mudou de dire\xc3\xa7\xc3\xa3o.\n"
            "N\xc3\xa3o sei explicar o que senti.\n\n"
            "Mas eu sei o que ouvi.\n"
            "E n\xc3\xa3o era vento.\n\n"
            "\xe2\x80\x94 Eleanor");
    }

    // Quintal grande: iluminação natural (simula céu externo com mais raio)
    m_lights.push_back({ {320.f, 144.f}, 90.f, 5.f, 1.8f, 0.0f });
    m_lights.push_back({ {160.f, 300.f}, 80.f, 4.f, 2.0f, 1.5f });
    m_lights.push_back({ {480.f, 300.f}, 80.f, 4.f, 2.2f, 3.0f });
    m_lights.push_back({ {320.f, 420.f}, 75.f, 3.f, 1.9f, 2.0f });
}

void Game::setupCozinha() {
    // Esquerda → sala de estar (col 0, rows 5-9 → pixel x=0, y=80-144)
    transitions.push_back({ {0.f, 80.f, 16.f, 64.f}, "sala_estar", {384.f, 120.f}, true });

    // Página 3: Registro de 1891 (placeholder, ajustar conforme mapa)
    if (pageItemTex.getSize().x == 0)
        pageItemTex.loadFromFile("assets/maps/sprites/items_book.png");
    sf::IntRect pr(0, 0, 32, 32);
    auto savedIt = m_presentItems.find("cozinha");
    auto itemOk  = [&](sf::Vector2f pos) -> bool {
        if (savedIt == m_presentItems.end()) return true;
        return savedIt->second.count({(int)std::round(pos.x), (int)std::round(pos.y)}) > 0;
    };
    if (itemOk({200.f, 120.f})) {
        items.addItem(ItemType::Page, {200.f, 120.f}, pageItemTex, pr, 999.f,
            "Registro de Constru\xc3\xa7\xc3\xa3o \xe2\x80\x94 1891",
            "Ordem do Sr. Voss:\n"
            "Selar o porão sul com argamassa\n"
            "e cal virgem. Motivo n\xc3\xa3o informado.\n\n"
            "Trabalhadores recusaram entrar\n"
            "ap\xc3\xb3s o segundo dia.\n"
            "Diziam ouvir vozes no interior\n"
            "das paredes.\n\n"
            "Trabalho conclu\xc3\xad" "do por m\xc3\xad" "m\n"
            "e Aldric, \xc3\xba" "nicos que ficaram.\n\n"
            "N\xc3\xa3o voltarei a este lugar.\n\n"
            "\xe2\x80\x94 H. Brenner, mestre de obras");
    }

    m_lights.push_back({ {176.f,  80.f}, 55.f, 10.f, 2.3f, 0.0f });
    m_lights.push_back({ {176.f, 176.f}, 50.f,  8.f, 2.1f, 1.8f });
}

void Game::setupPorao() {
    // Topo → hall principal (cols 11-14, row 0 = x 176-224, y 0)
    transitions.push_back({ {176.f, 0.f, 64.f, 32.f}, "hall_principal", {0.f, 240.f}, true });
    // Direita → porão fundo (col 25, rows 6-11 = x 400, y 96-176)
    transitions.push_back({ {400.f, 96.f, 16.f, 80.f}, "porao_fundo", {40.f, 120.f}, true });

    // Thomas — posição placeholder (ajustar conforme mapa do porão)
    npcTexThomas.loadFromFile("assets/maps/sprites/npcs/thomas/thomas.png");
    sf::IntRect npcRect(16, 0, 16, 24);
    NPC* thomas = new NPC("Thomas", 120.f, 160.f, npcTexThomas, npcRect);
    thomas->setColor(sf::Color(180, 180, 180, 200));
    thomas->addOption("Onde est\xc3\xa1 a chave para sair?", {
        {"Voc\xc3\xaa",   "Thomas, preciso sair. Onde est\xc3\xa1 a chave?",                            true},
        {"Thomas", "No dep\xc3\xb3" "sito. Al\xc3\xa9m do corredor.",                                   false},
        {"Thomas", "Atr\xc3\xa1s do espelho quebrado. Mas v\xc3\xa1 preparado.",                        false},
        {"Thomas", "Ela acorda quando voc\xc3\xaa est\xc3\xa1 prestes a sair. N\xc3\xa3o olhe para tr\xc3\xa1s.", false},
    });
    thomas->addOption("Tem algum lugar seguro?", {
        {"Voc\xc3\xaa",   "Existe algum lugar seguro nesta mans\xc3\xa3o?",                             true},
        {"Thomas", "Seguro? Aqui n\xc3\xa3o existe isso.",                                              false},
        {"Thomas", "Mas as sombras hesitam antes de entrar aqui. O por\xc3\xa3o as confunde.",          false},
    });
    npcs.push_back(thomas);

    // Página 4 — Anotações de Thomas (placeholder pos, ajustar conforme mapa)
    if (pageItemTex.getSize().x == 0)
        pageItemTex.loadFromFile("assets/maps/sprites/items_book.png");
    sf::IntRect pr(0, 0, 32, 32);
    auto savedIt = m_presentItems.find("porao");
    auto itemOk  = [&](sf::Vector2f pos) -> bool {
        if (savedIt == m_presentItems.end()) return true;
        return savedIt->second.count({(int)std::round(pos.x), (int)std::round(pos.y)}) > 0;
    };
    if (itemOk({80.f, 120.f})) {
        items.addItem(ItemType::Page, {80.f, 120.f}, pageItemTex, pr, 999.f,
            "Anota\xc3\xa7\xc3\xb5" "es de Thomas \xe2\x80\x94 Sem Data",
            "Observei por semanas.\n\n"
            "As sombras evitam o por\xc3\xa3o.\n"
            "H\xc3\xa1 algo no espelho antigo que as\n"
            "rep\xc3\xaa" "le \xe2\x80\x94 talvez o reflexo as confunda,\n"
            "talvez seja outra coisa. N\xc3\xa3o importa.\n"
            "O fato \xc3\xa9: elas n\xc3\xa3o entram.\n\n"
            "Escondi a chave da sa\xc3\xad" "da principal\n"
            "atr\xc3\xa1s do espelho. \xc3\x89 o \xc3\xba" "nico lugar\n"
            "que posso garantir que estar\xc3\xa1 l\xc3\xa1.\n\n"
            "Se voc\xc3\xaa est\xc3\xa1 lendo isso, chegou at\xc3\xa9\n"
            "aqui. Isso j\xc3\xa1 diz algo sobre voc\xc3\xaa.\n\n"
            "V\xc3\xa1 ao dep\xc3\xb3" "sito. Pegue a chave.\n"
            "N\xc3\xa3o pare para entender.\n"
            "N\xc3\xa3o olhe para tr\xc3\xa1s.\n"
            "Apenas saia.");
    }

    m_lights.push_back({ {208.f,  32.f}, 50.f, 8.f, 2.3f, 0.0f });
    m_lights.push_back({ { 80.f, 160.f}, 45.f, 7.f, 2.6f, 1.5f });
    m_lights.push_back({ {320.f, 160.f}, 45.f, 7.f, 2.4f, 2.8f });
}

void Game::setupPoraoFundo() {
    // Esquerda → porão (col 0, rows 5-9 = x 0, y 80-144)
    transitions.push_back({ {0.f, 80.f, 16.f, 80.f}, "porao", {384.f, 120.f}, true });
    // Direita → depósito (col 23, rows 5-9 = x 368, y 80-144)
    transitions.push_back({ {368.f, 80.f, 16.f, 80.f}, "deposito", {24.f, 96.f}, true });

    if (pageItemTex.getSize().x == 0)
        pageItemTex.loadFromFile("assets/maps/sprites/items_book.png");
    sf::IntRect pr(0, 0, 32, 32);
    auto savedIt = m_presentItems.find("porao_fundo");
    auto itemOk  = [&](sf::Vector2f pos) -> bool {
        if (savedIt == m_presentItems.end()) return true;
        return savedIt->second.count({(int)std::round(pos.x), (int)std::round(pos.y)}) > 0;
    };
    // Página 5 — A Última Mensagem (aciona o Boss ao ser coletada)
    if (itemOk({192.f, 128.f})) {
        items.addItem(ItemType::Page, {192.f, 128.f}, pageItemTex, pr, 999.f,
            "A \xc3\x9a" "ltima Mensagem \xe2\x80\x94 M.V.",
            "Encontrei as p\xc3\xa1ginas. Falei com todos.\n"
            "Tinha tudo que precisava.\n\n"
            "A sa\xc3\xad" "da estava a dez passos.\n"
            "Senti o ar frio do lado de fora.\n"
            "Vi a luz l\xc3\xa1 fora.\n\n"
            "Ouvi algo atr\xc3\xa1s de mim.\n"
            "Um som que n\xc3\xa3o era vento.\n"
            "N\xc3\xa3o era madeira rangendo.\n\n"
            "Olhei para tr\xc3\xa1s.\n\n"
            "Ela estava l\xc3\xa1.\n"
            "Sempre esteve.\n"
            "S\xc3\xb3 estava esperando eu perceber.\n\n"
            "Se voc\xc3\xaa encontrar isso \xe2\x80\x94\n"
            "n\xc3\xa3o cometa o mesmo erro.\n\n"
            "\xe2\x80\x94 M.V., novembro de 1987");
    }

    m_lights.push_back({ {192.f,  48.f}, 40.f, 6.f, 2.5f, 0.0f });
    m_lights.push_back({ { 80.f, 160.f}, 38.f, 5.f, 2.8f, 1.2f });
    m_lights.push_back({ {300.f, 160.f}, 38.f, 5.f, 2.2f, 2.5f });
}

void Game::setupDeposito() {
    // Esquerda → porão fundo (col 0, rows 4-8 = x 0, y 64-128)
    transitions.push_back({ {0.f, 64.f, 16.f, 80.f}, "porao_fundo", {352.f, 120.f}, true });
    // Direita → corredor de saída (col 19, rows 4-8 = x 304, y 64-128)
    transitions.push_back({ {304.f, 64.f, 16.f, 80.f}, "corredor_saida", {32.f, 64.f}, true });

    // Chave (placeholder pos, ajustar conforme mapa)
    if (keyItemTex.getSize().x == 0)
        keyItemTex.loadFromFile("assets/tilesets/PropsV2.png");
    sf::IntRect keyRect(0, 16, 16, 16);
    auto savedIt = m_presentItems.find("deposito");
    auto itemOk  = [&](sf::Vector2f pos) -> bool {
        if (savedIt == m_presentItems.end()) return true;
        return savedIt->second.count({(int)std::round(pos.x), (int)std::round(pos.y)}) > 0;
    };
    if (itemOk({160.f, 120.f}))
        items.addItem(ItemType::Key, {160.f, 120.f}, keyItemTex, keyRect);

    m_lights.push_back({ {160.f, 80.f},  55.f, 10.f, 2.4f, 0.0f });
    m_lights.push_back({ {160.f, 160.f}, 40.f,  8.f, 2.7f, 1.9f });
}

void Game::setupCorredorSaida() {
    // Esquerda → depósito (col 0, rows 3-6 = x 0, y 48-96)
    transitions.push_back({ {0.f, 48.f, 16.f, 64.f}, "deposito", {288.f, 96.f}, true });
    // Porta de saída no final do corredor (cols 26-27, rows 2-7 = x 416, y 32-112)
    doors.push_back({ {416.f, 32.f, 32.f, 96.f}, Door::Kind::Exit });

    // Iluminação esparsa — corredor deve parecer opressivo
    m_lights.push_back({ { 80.f, 80.f}, 40.f, 6.f, 2.8f, 0.0f });
    m_lights.push_back({ {224.f, 80.f}, 38.f, 5.f, 3.0f, 1.8f });
    m_lights.push_back({ {368.f, 80.f}, 35.f, 4.f, 2.5f, 3.2f });
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
    m_bossSpawned = false;

    mainMenuOption  = 0;
    characterOption = 0;

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

            if ((sf::Keyboard::isKeyPressed(sf::Keyboard::Up) || sf::Keyboard::isKeyPressed(sf::Keyboard::W)) && !keyUpPressed) {
                mainMenuOption = (mainMenuOption == 0) ? 1 : 0;
                keyUpPressed = true;
            }
            if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Up) && !sf::Keyboard::isKeyPressed(sf::Keyboard::W))
                keyUpPressed = false;

            if ((sf::Keyboard::isKeyPressed(sf::Keyboard::Down) || sf::Keyboard::isKeyPressed(sf::Keyboard::S)) && !keyDownPressed) {
                mainMenuOption = (mainMenuOption == 1) ? 0 : 1;
                keyDownPressed = true;
            }
            if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Down) && !sf::Keyboard::isKeyPressed(sf::Keyboard::S))
                keyDownPressed = false;

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Enter) && !keyEnterPressed) {
                if (mainMenuOption == 0) {
                    m_introPhase     = 1;
                    m_introOverlay   = 1.f;
                    m_introFadingOut = false;
                    characterOption  = 0;
                    keyUpPressed     = false;
                    keyDownPressed   = false;
                    state = GameState::Intro;
                    audio.playMusic(MusicTrack::Intro);
                } else {
                    window.close();
                }
                keyEnterPressed = true;
            }
            break;

        case GameState::Intro:
            updateIntro(dt);
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
    updateObjPopup(dt);
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
            case ItemType::Page: {
                player.addPage();
                audio.playSfx(SfxId::PageCollect);
                if (m_currentRoom == "area_externa" && !m_bossSpawned) {
                    m_bossSpawned = true;
                    sf::Vector2f bp = player.getPosition();
                    bp.x -= 300.f;
                    enemies.push_back(new Boss(bp.x, bp.y));
                    eventQueue.enqueue("Algo despertou. Fuja agora.");
                    audio.playMusic(MusicTrack::Boss);
                }
                {
                    std::string prog = std::to_string(player.getDiaryPages()) +
                                       " de " + std::to_string(totalPages) +
                                       " p\xc3\xa1" "ginas coletadas";
                    std::string ltU  = loreTitle.empty()
                                       ? "P\xc3\xa1" "gina do Di\xc3\xa1rio"
                                       : loreTitle;
                    showObjPopup(
                        sf::String::fromUtf8(
                            std::string("P\xc3\x81GINA DO DI\xc3\x81RIO").begin(),
                            std::string("P\xc3\x81GINA DO DI\xc3\x81RIO").end()),
                        sf::String::fromUtf8(ltU.begin(), ltU.end()),
                        sf::String::fromUtf8(prog.begin(), prog.end()),
                        sf::Color(255, 200, 55),
                        &pageItemTex,
                        sf::IntRect(0, 0, 32, 32));
                }
                if (!loreBody.empty()) {
                    pageReader.open(loreTitle, loreBody);
                    audio.playSfx(SfxId::BookOpen, 75.f);
                }
                break;
            }
            case ItemType::Lamp:
                player.addLantern();
                audio.playSfx(SfxId::ItemCollect);
                eventQueue.enqueue("Lamparina de sal coletada  [" +
                    std::to_string(player.getSaltLanterns()) + " lamparinas]");
                break;
            case ItemType::Key: {
                player.collectKey();
                audio.playSfx(SfxId::ItemCollect);
                std::string kn = "Chave da Mans\xc3\xa3o Voss";
                std::string ks = "Encontre a sa\xc3\xad" "da \xe2\x86\x92";
                showObjPopup(
                    sf::String::fromUtf8(
                        std::string("CHAVE ENCONTRADA").begin(),
                        std::string("CHAVE ENCONTRADA").end()),
                    sf::String::fromUtf8(kn.begin(), kn.end()),
                    sf::String::fromUtf8(ks.begin(), ks.end()),
                    sf::Color(80, 215, 120),
                    &keyItemTex,
                    sf::IntRect(0, 16, 16, 16));
                break;
            }
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

        m_doorNearby    = true;
        m_doorPromptPos = (t.promptPos.x != 0.f || t.promptPos.y != 0.f)
            ? t.promptPos
            : sf::Vector2f{ t.trigger.left + t.trigger.width * 0.5f,
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
        case GameState::Intro:    renderIntro();    break;
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
    // Gradiente de fundo: azul-escuro puro no topo, levemente mais quente embaixo
    {
        sf::VertexArray bg(sf::Quads, 4);
        bg[0] = {{0.f, 0.f},   sf::Color(5, 7, 14)};
        bg[1] = {{800.f, 0.f}, sf::Color(5, 7, 14)};
        bg[2] = {{800.f, 600.f}, sf::Color(12, 11, 20)};
        bg[3] = {{0.f,   600.f}, sf::Color(12, 11, 20)};
        window.draw(bg);
    }
    // Partículas flutuantes
    for (const auto& p : dustParticles) {
        sf::CircleShape dot(1.5f);
        dot.setPointCount(8);
        dot.setPosition(p.position);
        dot.setFillColor(sf::Color(210, 210, 195, static_cast<sf::Uint8>(p.alpha * 0.42f)));
        window.draw(dot);
    }

    // Layout fixo — não depende de getGlobalBounds().height que varia com a fonte
    const float SEP_Y  = 286.f;
    const float OPT1_Y = 348.f;
    const float OPT_GAP = 74.f;
    const float HINT_Y = 508.f;

    float flicker = 0.92f + 0.08f * std::sin(flickerTimer * 0.7f);
    float breathe = 0.5f  + 0.5f  * std::sin(flickerTimer * 0.9f);

    // Aura difusa atrás do título (retângulo suave com alpha baixo)
    {
        sf::FloatRect tb = titleText.getGlobalBounds();
        float cx = tb.left + tb.width * 0.5f;
        float cy = tb.top  + tb.height * 0.5f;

        // Halo externo grande
        sf::RectangleShape auraOuter({tb.width + 140.f, tb.height + 100.f});
        auraOuter.setOrigin((tb.width + 140.f) * 0.5f, (tb.height + 100.f) * 0.5f);
        auraOuter.setPosition(cx, cy);
        auraOuter.setFillColor(sf::Color(50, 55, 80, static_cast<sf::Uint8>(10.f + 8.f * breathe)));
        window.draw(auraOuter);

        // Halo interno mais definido
        sf::RectangleShape auraInner({tb.width + 60.f, tb.height + 40.f});
        auraInner.setOrigin((tb.width + 60.f) * 0.5f, (tb.height + 40.f) * 0.5f);
        auraInner.setPosition(cx, cy);
        auraInner.setFillColor(sf::Color(80, 85, 120, static_cast<sf::Uint8>(14.f + 10.f * breathe)));
        window.draw(auraInner);

        // 8 cópias em anel a 2px de raio — cria bloom suave
        for (int i = 0; i < 8; ++i) {
            float angle = i * (3.14159f / 4.f);
            sf::Text glowCopy = titleText;
            glowCopy.move(2.f * std::cos(angle), 2.f * std::sin(angle));
            glowCopy.setFillColor(sf::Color(95, 100, 140, static_cast<sf::Uint8>(16.f + 8.f * breathe)));
            window.draw(glowCopy);
        }
    }

    // Sombra do título
    sf::Text titleShadow = titleText;
    titleShadow.move(0.f, 4.f);
    titleShadow.setFillColor(sf::Color(0, 0, 0, 90));
    window.draw(titleShadow);

    // Título principal com flicker
    sf::Text titleCopy = titleText;
    titleCopy.setFillColor(sf::Color(128, 130, 145, static_cast<sf::Uint8>(215.f * flicker)));
    window.draw(titleCopy);

    // Linha separadora — posição FIXA
    sf::RectangleShape sep(sf::Vector2f(260.f, 1.f));
    sep.setPosition(400.f - 130.f, SEP_Y);
    sep.setFillColor(sf::Color(80, 82, 95, 130));
    window.draw(sep);

    // Opções de menu
    float pulse = 0.55f + 0.45f * std::sin(flickerTimer * 3.f);
    sf::Uint8 selAlpha = static_cast<sf::Uint8>(200 + 55 * pulse);

    struct MenuItem { const char* label; int idx; sf::Color selCol; };
    MenuItem items[] = {
        { "ADENTRAR A MANS\xc3\x83O", 0, sf::Color(235, 235, 245, selAlpha) },
        { "SAIR",                      1, sf::Color(236, 155, 155, selAlpha) }
    };

    float optY = OPT1_Y;
    for (const auto& item : items) {
        bool sel = (mainMenuOption == item.idx);

        sf::Text opt;
        opt.setFont(m_cinzel);
        opt.setCharacterSize(20);
        opt.setLetterSpacing(2.0f);
        std::string ls = item.label;
        opt.setString(sf::String::fromUtf8(ls.begin(), ls.end()));
        opt.setFillColor(sel ? item.selCol : sf::Color(105, 108, 122, 185));
        float ox = 400.f - opt.getGlobalBounds().width / 2.f;
        opt.setPosition(ox, optY);
        window.draw(opt);

        if (sel) {
            sf::Text arr;
            arr.setFont(font);
            arr.setCharacterSize(20);
            arr.setFillColor(item.selCol);
            std::string la = "\xe2\x80\xba", ra = "\xe2\x80\xb9";
            arr.setString(sf::String::fromUtf8(la.begin(), la.end()));
            arr.setPosition(ox - arr.getGlobalBounds().width - 12.f, optY + 1.f);
            window.draw(arr);
            arr.setString(sf::String::fromUtf8(ra.begin(), ra.end()));
            arr.setPosition(ox + opt.getGlobalBounds().width + 12.f, optY + 1.f);
            window.draw(arr);
        }

        optY += OPT_GAP;
    }

    // Dica de controles
    sf::Text hint;
    hint.setFont(font);
    hint.setCharacterSize(13);
    hint.setFillColor(sf::Color(68, 70, 82, 175));
    std::string hs = "W / S  mover   \xc2\xb7   ENTER  confirmar";
    hint.setString(sf::String::fromUtf8(hs.begin(), hs.end()));
    hint.setPosition(400.f - hint.getGlobalBounds().width / 2.f, HINT_Y);
    window.draw(hint);

    // Rodapé
    sf::Text footer;
    footer.setFont(font);
    footer.setCharacterSize(12);
    footer.setFillColor(sf::Color(52, 55, 63, 180));
    std::string fs = "MANS\xc3\x83O VOSS \xc2\xb7 1987";
    footer.setString(sf::String::fromUtf8(fs.begin(), fs.end()));
    footer.setPosition(400.f - footer.getGlobalBounds().width / 2.f, 568.f);
    window.draw(footer);

    drawVignette(sf::Color::Black);
}

void Game::updateIntro(float dt) {

    //pula toda a intro com ESC
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape) && !keyEscPressed) {
        m_introPhase     = 7; 
        m_introOverlay   = 0.f;
        m_introFadingOut = false;
        characterOption  = 0;
        keyEscPressed    = true;
        return;
    }
    if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) keyEscPressed = false;


    constexpr float FADE = 0.55f;

    flickerTimer += dt;
    for (auto& p : dustParticles) {
        p.position.y -= p.speed * dt;
        p.position.x += std::sin(flickerTimer * p.oscillationSpeed) * 5.f * dt;
        if (p.position.y < -10.f) {
            p.position.y = 610.f;
            p.position.x = std::rand() % 800;
        }
    }

    if (!m_introFadingOut) {
        if (m_introOverlay > 0.f) {
            m_introOverlay -= dt / FADE;
            if (m_introOverlay < 0.f) m_introOverlay = 0.f;
        }

        if (m_introPhase <= 6 && m_introOverlay <= 0.f) {
            if ((sf::Keyboard::isKeyPressed(sf::Keyboard::Enter) ||
                 sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) && !keyEnterPressed) {
                m_introFadingOut = true;
                keyEnterPressed  = true;
            }
        }

        if (m_introPhase == 4 && m_introOverlay <= 0.f) {
            m_gateTimer += dt;
            if (m_gateTimer > 0.9f)
                m_gateProgress = std::min(1.f, m_gateProgress + dt * 0.35f);
        }

        if (m_introPhase == 7 && m_introOverlay <= 0.f) {
            if ((sf::Keyboard::isKeyPressed(sf::Keyboard::Left) ||
                 sf::Keyboard::isKeyPressed(sf::Keyboard::A)) && !keyUpPressed) {
                characterOption = 0;
                keyUpPressed    = true;
            }
            if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Left) &&
                !sf::Keyboard::isKeyPressed(sf::Keyboard::A))
                keyUpPressed = false;

            if ((sf::Keyboard::isKeyPressed(sf::Keyboard::Right) ||
                 sf::Keyboard::isKeyPressed(sf::Keyboard::D)) && !keyDownPressed) {
                characterOption = 1;
                keyDownPressed  = true;
            }
            if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Right) &&
                !sf::Keyboard::isKeyPressed(sf::Keyboard::D))
                keyDownPressed = false;

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Enter) && !keyEnterPressed) {
                selectedSkin     = characterOption;
                m_introFadingOut = true;
                keyEnterPressed  = true;
            }
        }

        if (m_introPhase == 8 && m_introOverlay <= 0.f) {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Enter) && !keyEnterPressed) {
                m_introFadingOut = true;
                keyEnterPressed  = true;
            }
        }
    } else {
        m_introOverlay += dt / FADE;
        if (m_introOverlay >= 1.f) {
            m_introOverlay   = 1.f;
            m_introFadingOut = false;
            m_introPhase++;
            m_gateProgress   = 0.f;
            m_gateTimer      = 0.f;

            if (m_introPhase > 8) {
                player.load(selectedSkin == 1 ? "assets/maps/sprites/player/player_f.png"
                                              : "assets/maps/sprites/player/player_m.png");
                state = GameState::Playing;
                audio.playMusic(MusicTrack::Explore);
            }
        }
    }

    if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Enter) &&
        !sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
        keyEnterPressed = false;
}

void Game::renderIntro() {
    const float W = 800.f, H = 600.f;

    auto filledRect = [&](float x, float y, float w, float h, sf::Color c) {
        sf::RectangleShape r({w, h});
        r.setPosition(x, y);
        r.setFillColor(c);
        window.draw(r);
    };
    auto tri3 = [&](sf::Vector2f a, sf::Vector2f b, sf::Vector2f c2, sf::Color col) {
        sf::ConvexShape t(3);
        t.setPoint(0, a); t.setPoint(1, b); t.setPoint(2, c2);
        t.setFillColor(col);
        window.draw(t);
    };

    auto drawMansion = [&](float scale, float mtopY) {
        float mw = 520.f * scale, mh = 400.f * scale;
        float ml = 400.f - mw * 0.5f, mt = mtopY;
        sf::Color mc(3, 5, 9);    // torres/chaminés (mais escuro)
        sf::Color mb(5, 7, 12);   // corpo principal
        sf::Color mrf(4, 6, 10);  // telhado

        auto mr2 = [&](float rx, float ry, float rw, float rh, sf::Color c) {
            filledRect(ml+rx, mt+ry, rw, rh, c);
        };

        // --- Torres laterais ---
        // Torre esquerda (mais alta)
        mr2(mw*0.02f, mh*0.22f, mw*0.13f, mh*0.78f, mc);
        // Telhado torre esq (triângulo pontiagudo)
        tri3({ml+mw*0.085f, mt+mh*0.02f},
             {ml+mw*0.15f,  mt+mh*0.24f},
             {ml+mw*0.02f,  mt+mh*0.24f}, mc);
        // Torre direita
        mr2(mw*0.85f, mh*0.22f, mw*0.13f, mh*0.78f, mc);
        // Telhado torre dir
        tri3({ml+mw*0.915f, mt+mh*0.02f},
             {ml+mw*0.98f,  mt+mh*0.24f},
             {ml+mw*0.85f,  mt+mh*0.24f}, mc);

        // --- Corpo principal ---
        mr2(mw*0.15f, mh*0.46f, mw*0.70f, mh*0.54f, mb);

        // --- TELHADO PRINCIPAL (trapézio — o "telhado" que faltava) ---
        sf::ConvexShape telhado(4);
        telhado.setPoint(0, {ml + mw*0.22f, mt + mh*0.28f}); // canto sup esq
        telhado.setPoint(1, {ml + mw*0.78f, mt + mh*0.28f}); // canto sup dir
        telhado.setPoint(2, {ml + mw*0.85f, mt + mh*0.46f}); // canto inf dir
        telhado.setPoint(3, {ml + mw*0.15f, mt + mh*0.46f}); // canto inf esq
        telhado.setFillColor(mrf);
        window.draw(telhado);

        // Calha/beira do telhado (linha mais escura no topo do trapézio)
        mr2(mw*0.22f, mh*0.27f, mw*0.56f, mh*0.025f, sf::Color(2,3,7));

        // --- Dormer/empena central (sobre o telhado) ---
        tri3({ml+mw*0.50f, mt+mh*0.12f},
             {ml+mw*0.63f, mt+mh*0.30f},
             {ml+mw*0.37f, mt+mh*0.30f}, mc);

        // --- Chaminés ---
        mr2(mw*0.28f,  -mh*0.05f, mw*0.035f, mh*0.22f, mc);
        mr2(mw*0.28f,  -mh*0.06f, mw*0.055f, mh*0.025f, mc); // chapéu esq
        mr2(mw*0.65f,  -mh*0.07f, mw*0.035f, mh*0.24f, mc);
        mr2(mw*0.645f, -mh*0.08f, mw*0.055f, mh*0.025f, mc); // chapéu dir
        mr2(mw*0.47f,  -mh*0.01f, mw*0.022f, mh*0.16f, mc);  // chaminé central menor
        // Chaminés das torres
        mr2(mw*0.04f,  -mh*0.04f, mw*0.025f, mh*0.10f, mc);
        mr2(mw*0.90f,  -mh*0.04f, mw*0.025f, mh*0.10f, mc);

        // --- Platibanda/parapeito (conecta torres ao telhado) ---
        mr2(mw*0.15f, mh*0.44f, mw*0.70f, mh*0.03f, sf::Color(3,4,8, 200));

        // --- Portal / porta ---
        mr2(mw*0.42f, mh*0.78f, mw*0.16f, mh*0.22f, sf::Color(255,145,50, 55));
        mr2(mw*0.46f, mh*0.83f, mw*0.08f, mh*0.17f, sf::Color(5,3,1));
        // Arco da porta
        tri3({ml+mw*0.50f, mt+mh*0.80f},
             {ml+mw*0.54f, mt+mh*0.84f},
             {ml+mw*0.46f, mt+mh*0.84f}, sf::Color(255,140,45,45));

        // --- Janelas com flicker ---
        float f1 = 0.55f + 0.45f * std::sin(flickerTimer * 1.10f);
        float f2 = 0.55f + 0.45f * std::sin(flickerTimer * 1.40f + 1.2f);
        float f3 = 0.55f + 0.45f * std::sin(flickerTimer * 0.80f + 2.8f);
        float f4 = 0.55f + 0.45f * std::sin(flickerTimer * 1.65f + 0.4f);
        // Torres — âmbar
        mr2(mw*0.04f, mh*0.34f, mw*0.06f, mh*0.09f, sf::Color(255,178,92, sf::Uint8(185*f1)));
        mr2(mw*0.04f, mh*0.52f, mw*0.06f, mh*0.09f, sf::Color(255,178,92, sf::Uint8(110*f2)));
        mr2(mw*0.90f, mh*0.36f, mw*0.06f, mh*0.09f, sf::Color(255,178,92, sf::Uint8(195*f3)));
        mr2(mw*0.90f, mh*0.54f, mw*0.06f, mh*0.09f, sf::Color(255,178,92, sf::Uint8(90*f4)));
        // Corpo principal — frios (azul-cinza)
        mr2(mw*0.22f, mh*0.56f, mw*0.07f, mh*0.10f, sf::Color(120,150,200, 45));
        mr2(mw*0.38f, mh*0.56f, mw*0.07f, mh*0.10f, sf::Color(120,150,200, 35));
        mr2(mw*0.62f, mh*0.56f, mw*0.07f, mh*0.10f, sf::Color(255,178,92, sf::Uint8(105*f1)));
        // Dormer — janela pequena
        mr2(mw*0.47f, mh*0.17f, mw*0.06f, mh*0.09f, sf::Color(120,150,200, 40));
    };

    auto drawHills = [&](float horizY) {
        float pts[][2] = {
            {0,H},{0,horizY*0.62f},{W*0.05f,horizY*0.40f},{W*0.09f,horizY*0.57f},
            {W*0.14f,horizY*0.32f},{W*0.20f,horizY*0.54f},{W*0.24f,horizY*0.36f},
            {W*0.30f,horizY*0.58f},{W*0.36f,horizY*0.34f},{W*0.42f,horizY*0.56f},
            {W*0.48f,horizY*0.28f},{W*0.54f,horizY*0.52f},{W*0.60f,horizY*0.35f},
            {W*0.66f,horizY*0.57f},{W*0.72f,horizY*0.30f},{W*0.78f,horizY*0.55f},
            {W*0.84f,horizY*0.38f},{W*0.90f,horizY*0.58f},{W*0.95f,horizY*0.42f},
            {W,horizY*0.60f},{W,H}
        };
        sf::ConvexShape hills(21);
        for (int i = 0; i < 21; ++i) hills.setPoint(i, {pts[i][0], pts[i][1]});
        hills.setFillColor(sf::Color(5, 7, 12));
        window.draw(hills);
    };

    auto drawRain = [&]() {
        for (int i = 0; i < 80; ++i) {
            float seed = i * 137.508f;
            float speed = 260.f + (i % 7) * 35.f;
            float xBase = std::fmod(seed * 3.7f, W);
            float yBase = std::fmod(flickerTimer * speed + seed * 13.3f, H + 80.f) - 40.f;
            sf::RectangleShape streak({1.f, 12.f + (i % 4) * 5.f});
            streak.setOrigin(0.5f, 0.f);
            streak.setPosition(xBase, yBase);
            streak.setRotation(-12.f);
            streak.setFillColor(sf::Color(140,155,175, sf::Uint8(25 + (i%5)*10)));
            window.draw(streak);
        }
    };

    // --- Phase 1: pure dark ---
    if (m_introPhase == 1) {
        filledRect(0.f, 0.f, W, H, sf::Color(7, 9, 15));
    }

    // --- Phases 2-3: mansion silhouette ---
    if (m_introPhase == 2 || m_introPhase == 3) {
        // Gradiente de céu: azul muito escuro em cima, ligeiramente mais quente no horizonte
        {
            sf::VertexArray sky(sf::Quads, 4);
            sky[0] = {{0.f,0.f},  sf::Color(4, 6, 14)};
            sky[1] = {{W, 0.f},   sf::Color(4, 6, 14)};
            sky[2] = {{W, H},     sf::Color(10, 13, 24)};
            sky[3] = {{0.f, H},   sf::Color(10, 13, 24)};
            window.draw(sky);
        }
        float mr = 34.f, mx = W*0.74f, my = H*0.11f;
        // Halo externo difuso da lua (3 camadas)
        for (int gi = 3; gi >= 1; --gi) {
            float gr = mr * (1.f + gi * 0.7f);
            sf::CircleShape g(gr);
            g.setPointCount(64);
            g.setFillColor(sf::Color(160,175,210, sf::Uint8(6 * gi)));
            g.setPosition(mx-gr, my-gr);
            window.draw(g);
        }
        sf::CircleShape moon(mr);
        moon.setPointCount(64);
        moon.setFillColor(sf::Color(222,224,232));
        moon.setPosition(mx-mr, my-mr);
        window.draw(moon);
        float horizY = H * (m_introPhase == 2 ? 0.55f : 0.50f);
        drawHills(horizY);
        filledRect(0.f, horizY, W, H - horizY, sf::Color(4,5,9));
        float sc = (m_introPhase == 2) ? 0.52f : 0.65f;
        drawMansion(sc, H * (m_introPhase == 2 ? 0.28f : 0.22f));
        if (m_introPhase == 3) {
            float tx = 400.f - 520.f*0.65f*0.5f - 55.f;
            filledRect(tx, H*0.42f, 9.f, H*0.44f, sf::Color(3,4,8));
            sf::RectangleShape b1({88.f, 5.f});
            b1.setOrigin(0.f,2.5f); b1.setPosition(tx+5.f, H*0.42f+70.f);
            b1.setRotation(-36.f); b1.setFillColor(sf::Color(3,4,8));
            window.draw(b1);
            sf::RectangleShape b2({70.f, 4.f});
            b2.setOrigin(0.f,2.f); b2.setPosition(tx+5.f, H*0.42f+130.f);
            b2.setRotation(30.f); b2.setFillColor(sf::Color(3,4,8));
            window.draw(b2);
        }
        float fogX = std::sin(flickerTimer * 0.25f) * 18.f;
        filledRect(-50.f+fogX, H*0.72f, W+100.f, 110.f, sf::Color(120,130,150,20));
        filledRect(-50.f-fogX, H*0.80f, W+100.f,  80.f, sf::Color(110,120,140,16));
        drawRain();
    }

    // --- Phase 4: gate opening ---
    if (m_introPhase == 4) {
        {
            sf::VertexArray sky(sf::Quads, 4);
            sky[0] = {{0.f,0.f},  sf::Color(4, 6, 14)};
            sky[1] = {{W, 0.f},   sf::Color(4, 6, 14)};
            sky[2] = {{W, H},     sf::Color(9, 12, 22)};
            sky[3] = {{0.f, H},   sf::Color(9, 12, 22)};
            window.draw(sky);
        }
        float mr = 28.f, mx = W*0.64f, my = H*0.11f;
        for (int gi = 3; gi >= 1; --gi) {
            float gr = mr * (1.f + gi * 0.7f);
            sf::CircleShape g(gr);
            g.setPointCount(64);
            g.setFillColor(sf::Color(160,175,210, sf::Uint8(5 * gi)));
            g.setPosition(mx-gr, my-gr);
            window.draw(g);
        }
        sf::CircleShape moon(mr);
        moon.setPointCount(64);
        moon.setFillColor(sf::Color(215,218,226));
        moon.setPosition(mx-mr, my-mr);
        window.draw(moon);
        drawHills(H * 0.48f);
        filledRect(0.f, H*0.48f, W, H*0.52f, sf::Color(4,5,9));
        drawMansion(0.30f, H*0.26f);

        // Moonlight beam
        float beamW = 90.f + m_gateProgress * 120.f;
        sf::ConvexShape beam(4);
        beam.setPoint(0, {W*0.5f-8.f, 0.f});
        beam.setPoint(1, {W*0.5f+8.f, 0.f});
        beam.setPoint(2, {W*0.5f+beamW*0.5f, H});
        beam.setPoint(3, {W*0.5f-beamW*0.5f, H});
        beam.setFillColor(sf::Color(160,185,230,20));
        window.draw(beam);

        // Stone pillars
        float pW = 78.f, pLeft = W*0.05f, pRight = W - pW - W*0.05f;
        sf::Color pDark(22,25,32);
        auto pillar = [&](float px) {
            filledRect(px, 0.f, pW, H, pDark);
            for (int i = 0; i < 22; ++i)
                filledRect(px, i*28.f, pW, 2.f, sf::Color(28,31,40,100));
            filledRect(px, 0.f, 3.f, H, sf::Color(55,60,72,70));
            filledRect(px+pW-3.f, 0.f, 3.f, H, sf::Color(10,12,18,70));
            filledRect(px-8.f, H*0.02f, pW+16.f, 28.f, sf::Color(42,46,56));
            tri3({px-8.f+(pW+16.f)*0.5f, H*0.02f-22.f},
                 {px-8.f+pW+16.f, H*0.02f},
                 {px-8.f, H*0.02f}, sf::Color(30,33,42));
        };
        pillar(pLeft);
        pillar(pRight);

        // Gate bars — abre de DENTRO para FORA (cada metade recua em direção ao seu pilar)
        float gateL = pLeft + pW, gateRight = pRight;
        float gateCenter = W*0.5f;
        float gateTop = H*0.08f, gateH = H*0.92f;
        float halfSpan = gateCenter - gateL;
        float prog = m_gateProgress;
        sf::Color barCol(11,12,18);

        // METADE ESQUERDA: dobradiça em gateL, borda direita recua de gateCenter → gateL
        float leftW = halfSpan * (1.f - prog);
        if (leftW > 1.f) {
            filledRect(gateL, gateTop,              leftW, 6.f, barCol);
            filledRect(gateL, gateTop+gateH*0.46f,  leftW, 5.f, barCol);
            filledRect(gateL, gateTop+gateH-6.f,    leftW, 6.f, barCol);
            for (int i = 0; i < 7; ++i) {
                float bx = gateL + (i / 6.f) * halfSpan;
                if (bx < gateL + leftW)
                    filledRect(bx - 3.f, gateTop, 6.f, gateH, barCol);
            }
        }

        // METADE DIREITA: dobradiça em gateRight, borda esquerda avança de gateCenter → gateRight
        float rightStart = gateCenter + halfSpan * prog;
        float rightW     = halfSpan * (1.f - prog);
        if (rightW > 1.f) {
            filledRect(rightStart, gateTop,              rightW, 6.f, barCol);
            filledRect(rightStart, gateTop+gateH*0.46f,  rightW, 5.f, barCol);
            filledRect(rightStart, gateTop+gateH-6.f,    rightW, 6.f, barCol);
            for (int i = 0; i < 7; ++i) {
                float bx = gateCenter + (i / 6.f) * halfSpan;
                if (bx >= rightStart)
                    filledRect(bx - 3.f, gateTop, 6.f, gateH, barCol);
            }
        }

        // Luz no vão central
        if (prog > 0.05f) {
            float gapW = halfSpan * 2.f * prog;
            filledRect(gateCenter - gapW * 0.5f, gateTop, gapW, gateH,
                       sf::Color(160,185,230, sf::Uint8(28*prog)));
        }
        drawRain();
    }

    // --- Phase 5: porta frontal da mansão (grande e elegante) ---
    if (m_introPhase == 5) {
        // Fundo noturno
        {
            sf::VertexArray sky(sf::Quads, 4);
            sky[0] = {{0.f, 0.f}, sf::Color(4, 6, 14)};
            sky[1] = {{W,   0.f}, sf::Color(4, 6, 14)};
            sky[2] = {{W,   H},   sf::Color(8, 10, 20)};
            sky[3] = {{0.f, H},   sf::Color(8, 10, 20)};
            window.draw(sky);
        }
        // Lua
        float mr = 22.f, mx = W*0.78f, my = H*0.10f;
        for (int gi = 3; gi >= 1; --gi) {
            float gr = mr * (1.f + gi * 0.7f);
            sf::CircleShape g(gr); g.setPointCount(64);
            g.setFillColor(sf::Color(160,175,210, sf::Uint8(5*gi)));
            g.setPosition(mx-gr, my-gr); window.draw(g);
        }
        { sf::CircleShape moon(mr); moon.setPointCount(64);
          moon.setFillColor(sf::Color(215,218,226));
          moon.setPosition(mx-mr, my-mr); window.draw(moon); }

        // Fachada de pedra (muro)
        filledRect(0.f, H*0.18f, W, H*0.82f, sf::Color(16, 18, 24));
        // Textura: linhas horizontais de pedra
        for (int i = 0; i < 22; ++i)
            filledRect(0.f, H*0.18f + i*22.f, W, 1.5f, sf::Color(22,25,33,80));

        // Porta: arco gótico central
        float dw = 130.f, dh = 260.f;
        float dx = W*0.5f - dw*0.5f, dy = H*0.18f + 30.f;
        sf::Color stoneCol(28, 31, 40);
        sf::Color doorCol(8, 6, 4);

        // Pilastras laterais
        filledRect(dx - 22.f, dy, 22.f, dh + 20.f, stoneCol);
        filledRect(dx + dw,   dy, 22.f, dh + 20.f, stoneCol);
        // Detalhes das pilastras (friso)
        filledRect(dx - 26.f, dy + dh*0.3f, 30.f, 6.f, sf::Color(35,38,48));
        filledRect(dx - 26.f, dy + dh*0.6f, 30.f, 6.f, sf::Color(35,38,48));
        filledRect(dx + dw - 4.f, dy + dh*0.3f, 30.f, 6.f, sf::Color(35,38,48));
        filledRect(dx + dw - 4.f, dy + dh*0.6f, 30.f, 6.f, sf::Color(35,38,48));

        // Arco (semi-círculo usando VertexArray TriangleFan)
        {
            const int SEGS = 24;
            sf::VertexArray arch(sf::TriangleFan, SEGS + 2);
            float archCx = W*0.5f, archCy = dy + dw*0.5f;
            float archR  = dw*0.5f + 12.f;
            arch[0] = {{archCx, archCy}, stoneCol};
            for (int i = 0; i <= SEGS; ++i) {
                float a = 3.14159f + 3.14159f * i / SEGS;
                arch[i+1] = {{archCx + archR * std::cos(a),
                               archCy + archR * std::sin(a)}, stoneCol};
            }
            window.draw(arch);
        }
        // Interior do arco (abertura)
        {
            const int SEGS = 24;
            sf::VertexArray archInner(sf::TriangleFan, SEGS + 2);
            float archCx = W*0.5f, archCy = dy + dw*0.5f;
            float archR  = dw*0.5f;
            archInner[0] = {{archCx, archCy}, doorCol};
            for (int i = 0; i <= SEGS; ++i) {
                float a = 3.14159f + 3.14159f * i / SEGS;
                archInner[i+1] = {{archCx + archR * std::cos(a),
                                   archCy + archR * std::sin(a)}, doorCol};
            }
            window.draw(archInner);
        }

        // Corpo da porta (parte retangular abaixo do arco)
        filledRect(dx, dy + dw*0.5f, dw, dh - dw*0.5f, doorCol);
        // Friso superior (verga)
        filledRect(dx - 22.f, dy + dw*0.5f - 6.f, dw + 44.f, 10.f, stoneCol);

        // Painéis decorativos da porta (2 painéis)
        sf::Color panelCol(12, 10, 7);
        filledRect(dx + 10.f, dy + dw*0.5f + 12.f, dw*0.5f - 14.f, dh*0.32f, panelCol);
        filledRect(dx + dw*0.5f + 4.f, dy + dw*0.5f + 12.f, dw*0.5f - 14.f, dh*0.32f, panelCol);
        filledRect(dx + 10.f, dy + dw*0.5f + dh*0.37f, dw*0.5f - 14.f, dh*0.32f, panelCol);
        filledRect(dx + dw*0.5f + 4.f, dy + dw*0.5f + dh*0.37f, dw*0.5f - 14.f, dh*0.32f, panelCol);

        // Divisória central da porta (linha vertical)
        filledRect(W*0.5f - 2.f, dy + dw*0.5f, 4.f, dh - dw*0.5f, sf::Color(6,5,3));

        // Aldrava / puxador (centro)
        float kx = W*0.5f - 6.f, ky = dy + dh*0.65f;
        filledRect(kx, ky, 5.f, 12.f, sf::Color(45,42,36));
        filledRect(kx + dw*0.5f + 1.f, ky, 5.f, 12.f, sf::Color(45,42,36));

        // Fanlight / janela semicircular acima do arco
        {
            const int SEGS = 16;
            sf::VertexArray fan(sf::TriangleFan, SEGS + 2);
            float fcx = W*0.5f, fcy = dy + dw*0.5f;
            float fr  = dw*0.5f - 8.f;
            fan[0] = {{fcx, fcy}, sf::Color(40,60,100,120)};
            for (int i = 0; i <= SEGS; ++i) {
                float a = 3.14159f + 3.14159f * i / SEGS;
                fan[i+1] = {{fcx + fr*std::cos(a), fcy + fr*std::sin(a)},
                             sf::Color(60,90,150,60)};
            }
            window.draw(fan);
            // Ferros radiais do fanlight
            for (int i = 1; i < SEGS; i += 4) {
                float a = 3.14159f + 3.14159f * i / SEGS;
                sf::RectangleShape rod({fr, 2.f});
                rod.setOrigin(0.f, 1.f);
                rod.setPosition(fcx, fcy);
                rod.setRotation(a * 180.f / 3.14159f);
                rod.setFillColor(sf::Color(22,24,30,180));
                window.draw(rod);
            }
        }

        // Lanternas de ferro em cada pilastra
        float lanternFlicker = 0.7f + 0.3f * std::sin(flickerTimer * 4.2f);
        for (int side = -1; side <= 1; side += 2) {
            float lx = W*0.5f + side * (dw*0.5f + 55.f);
            float ly = dy + dh*0.3f;
            // Corpo da lanterna
            filledRect(lx - 7.f, ly, 14.f, 22.f, sf::Color(20,22,28));
            // Chama
            sf::CircleShape flame(4.f * lanternFlicker);
            flame.setPointCount(32);
            flame.setFillColor(sf::Color(255,160,50, sf::Uint8(180*lanternFlicker)));
            flame.setPosition(lx - 4.f*lanternFlicker, ly - 4.f*lanternFlicker);
            window.draw(flame);
            // Halo
            sf::CircleShape halo(14.f * lanternFlicker);
            halo.setPointCount(32);
            halo.setFillColor(sf::Color(255,130,20, sf::Uint8(18*lanternFlicker)));
            halo.setPosition(lx - 14.f*lanternFlicker, ly - 14.f*lanternFlicker);
            window.draw(halo);
            // Haste da lanterna
            filledRect(lx - 1.5f, dy + dh*0.15f, 3.f, dh*0.14f, sf::Color(20,22,28));
        }

        // Degraus de pedra (3 degraus em perspectiva)
        sf::Color stepCol(20, 22, 30);
        for (int i = 0; i < 3; ++i) {
            float sw = dw + 44.f + i*28.f;
            filledRect(W*0.5f - sw*0.5f, dy + dh + i*12.f + 20.f, sw, 12.f,
                       sf::Color(stepCol.r + i*3, stepCol.g + i*3, stepCol.b + i*4));
        }

        // Névoa rasteira
        float fogX = std::sin(flickerTimer * 0.2f) * 12.f;
        filledRect(-40.f+fogX, H*0.74f, W+80.f, 80.f, sf::Color(110,120,145,14));
        filledRect(-40.f-fogX, H*0.82f, W+80.f, 60.f, sf::Color(100,110,135,10));
    }

    // --- Phase 6: A Criança no vestíbulo (interior sombrio) ---
    if (m_introPhase == 6) {
        filledRect(0.f, 0.f, W, H, sf::Color(5, 7, 12));

        // Corredor com perspectiva sugerida (fundo mais claro, bordas escuras)
        {
            sf::VertexArray corridor(sf::Quads, 4);
            corridor[0] = {{W*0.2f, H*0.6f}, sf::Color(10,12,18)};
            corridor[1] = {{W*0.8f, H*0.6f}, sf::Color(10,12,18)};
            corridor[2] = {{W*1.1f, H},      sf::Color(6, 8, 14)};
            corridor[3] = {{-W*0.1f, H},     sf::Color(6, 8, 14)};
            window.draw(corridor);
        }
        // Rodapé / piso
        filledRect(0.f, H*0.74f, W, H*0.26f, sf::Color(14,16,22));
        // Linha do horizonte do corredor
        filledRect(0.f, H*0.74f, W, 2.f, sf::Color(25,28,36,120));

        // Vela no chão — iluminação suave
        float cx3 = W*0.62f, cy3 = H*0.70f;
        float cf3 = 0.65f + 0.35f * std::sin(flickerTimer * 6.1f);
        for (int gi = 3; gi >= 1; --gi) {
            float gr = 55.f * cf3 * (0.5f + gi * 0.55f);
            sf::CircleShape cg(gr); cg.setPointCount(32);
            cg.setFillColor(sf::Color(255,120,15, sf::Uint8(8 * gi * cf3)));
            cg.setPosition(cx3-gr, cy3-gr); window.draw(cg);
        }
        { sf::CircleShape fl(5.f*cf3); fl.setPointCount(32);
          fl.setFillColor(sf::Color(255,190,80, sf::Uint8(200*cf3)));
          fl.setPosition(cx3-5.f*cf3, cy3-5.f*cf3); window.draw(fl); }
        filledRect(cx3-2.5f, cy3, 5.f, 15.f, sf::Color(190,185,165));

        // Silhueta de A Criança — pequena, ao fundo do corredor
        float childX = W*0.47f, childY = H*0.44f;
        float childScale = 0.62f;  // menor = parece estar no fundo
        float childPulse = 0.7f + 0.3f * std::sin(flickerTimer * 1.5f);
        sf::Uint8 ca = sf::Uint8(180 * childPulse);
        sf::Color childCol(10, 12, 18, ca);
        // Cabeça
        sf::CircleShape chead(16.f * childScale);
        chead.setPointCount(64);
        chead.setFillColor(childCol);
        chead.setPosition(childX - 16.f*childScale, childY);
        window.draw(chead);
        // Corpo
        sf::ConvexShape cbody(4);
        cbody.setPoint(0, {childX - 11.f*childScale, childY + 30.f*childScale});
        cbody.setPoint(1, {childX + 11.f*childScale, childY + 30.f*childScale});
        cbody.setPoint(2, {childX + 14.f*childScale, childY + 80.f*childScale});
        cbody.setPoint(3, {childX - 14.f*childScale, childY + 80.f*childScale});
        cbody.setFillColor(childCol);
        window.draw(cbody);

        // Aura etérea em volta da criança (suave)
        sf::CircleShape aura(40.f * childScale);
        aura.setPointCount(32);
        aura.setFillColor(sf::Color(120,130,170, sf::Uint8(12 * childPulse)));
        aura.setPosition(childX - 40.f*childScale, childY - 10.f*childScale);
        window.draw(aura);
    }

    // --- Dust particles (all phases) ---
    for (const auto& p : dustParticles) {
        sf::CircleShape dot(1.5f);
        dot.setPointCount(8);
        dot.setPosition(p.position);
        dot.setFillColor(sf::Color(210, 210, 195, static_cast<sf::Uint8>(p.alpha * 0.36f)));
        window.draw(dot);
    }

    // --- Phases 1-5: narrative text ---
    struct IntroPhaseText { const char* main; const char* sub; };
    static const IntroPhaseText phases[] = {
        {   // 1
            "Na noite de 14 de novembro de 1987,\na fam\xc3\xad" "lia Voss desapareceu.",
            "Eleanor, Thomas e a crian\xc3\xa7" "a \xe2\x80\x94 nenhum rastro."
        },
        {   // 2
            "O munic\xc3\xad" "pio te enviou como investigador\npara descobrir o que aconteceu.",
            "Uma fam\xc3\xad" "lia inteira. Sumida dentro de casa."
        },
        {   // 3
            "A Mans\xc3\xa3o Voss surge entre os pinheiros \xe2\x80\x94\nvelha, im\xc3\xb3vel, atenta.",
            "Erguida em 1891 sobre algo que jamais\ndeveria ter sido perturbado."
        },
        {   // 4
            "O port\xc3\xa3o de ferro cede com um rangido longo.",
            "Seja o que for que aconteceu \xc3\xa0 fam\xc3\xad" "lia Voss,\na resposta est\xc3\xa1 l\xc3\xa1 dentro."
        },
        {   // 5  (porta frontal da mansão)
            "A porta da mans\xc3\xa3o \xe2\x80\x94 alta, pesada, imponente.",
            "Voc\xc3\xaa a empurra. O ar l\xc3\xa1 dentro \xc3\xa9 frio como pedra."
        },
        {   // 6  (A Criança no vestíbulo)
            "No vest\xc3\xad" "bulo, uma voz de crian\xc3\xa7" "a ecoa sozinha.",
            "A crian\xc3\xa7" "a para. Olha para voc\xc3\xaa. E corre."
        }
    };

    if (m_introPhase >= 1 && m_introPhase <= 6) {
        const auto& t = phases[m_introPhase - 1];
        bool hasScene = (m_introPhase >= 2);
        float textY = hasScene ? H - 160.f : H * 0.38f;

        if (hasScene)
            filledRect(0.f, H - 195.f, W, 195.f, sf::Color(0,0,0,145));

        sf::Text mainTxt;
        mainTxt.setFont(font);
        mainTxt.setCharacterSize(22);
        mainTxt.setFillColor(sf::Color(196, 199, 207, 255));
        std::string ms = t.main;
        mainTxt.setString(sf::String::fromUtf8(ms.begin(), ms.end()));
        mainTxt.setPosition(400.f - mainTxt.getGlobalBounds().width / 2.f, textY);
        window.draw(mainTxt);

        // subY calculado após a altura real do mainTxt + espaço generoso
        float subY = textY + mainTxt.getGlobalBounds().height + 22.f;

        sf::Text subTxt;
        subTxt.setFont(font);
        subTxt.setCharacterSize(18);
        subTxt.setStyle(sf::Text::Italic);
        subTxt.setFillColor(sf::Color(128, 131, 140, 210));
        std::string ss = t.sub;
        subTxt.setString(sf::String::fromUtf8(ss.begin(), ss.end()));
        subTxt.setPosition(400.f - subTxt.getGlobalBounds().width / 2.f, subY);
        window.draw(subTxt);

        float hpulse = 0.4f + 0.6f * std::sin(flickerTimer * 2.1f);
        sf::Text hintEnter, hintEsc;

        hintEnter.setFont(font);
        hintEnter.setCharacterSize(14);
        hintEnter.setFillColor(sf::Color(174, 177, 186, static_cast<sf::Uint8>(60 + 140 * hpulse)));
        std::string he = "ENTER continuar \xe2\x96\xb8";
        hintEnter.setString(sf::String::fromUtf8(he.begin(), he.end()));
        hintEnter.setPosition(W - hintEnter.getGlobalBounds().width - 22.f, H - 58.f);
        window.draw(hintEnter);

        hintEsc.setFont(font);
        hintEsc.setCharacterSize(12);
        hintEsc.setFillColor(sf::Color(174, 177, 186, static_cast<sf::Uint8>(60 + 140 * hpulse)));
        std::string hs = "ESC pular introdu\xc3\xa7\xc3\xa3o";
        hintEsc.setString(sf::String::fromUtf8(hs.begin(), hs.end()));
        hintEsc.setPosition(W - hintEsc.getGlobalBounds().width - 22.f, H - 38.f);
        window.draw(hintEsc);
    }

    // --- Phase 7: character select ---
    if (m_introPhase == 7) {
        sf::Text selTitle;
        selTitle.setFont(m_cinzel);
        selTitle.setCharacterSize(20);
        selTitle.setLetterSpacing(2.2f);
        selTitle.setFillColor(sf::Color(157, 160, 170, 210));
        std::string st = "ESCOLHA SEU INVESTIGADOR";
        selTitle.setString(sf::String::fromUtf8(st.begin(), st.end()));
        selTitle.setPosition(400.f - selTitle.getGlobalBounds().width / 2.f, 68.f);
        window.draw(selTitle);

        filledRect(400.f - 90.f, 108.f, 180.f, 2.f, sf::Color(90,90,100,130));

        const float cW = 155.f, cH = 255.f, cY = 128.f;
        sf::Texture texM, texF;
        bool loaded = texM.loadFromFile("assets/maps/sprites/player/player_m.png") &&
                      texF.loadFromFile("assets/maps/sprites/player/player_f.png");

        auto drawCard = [&](float cx, int idx, sf::Color accent,
                             sf::Texture& tex, const char* name, const char* role) {
            bool sel = (characterOption == idx);
            sf::RectangleShape card({cW, cH});
            card.setPosition(cx - cW*0.5f, cY);
            card.setFillColor(sel ? sf::Color(accent.r,accent.g,accent.b,22)
                                  : sf::Color(255,255,255,6));
            card.setOutlineThickness(sel ? 2.f : 1.f);
            card.setOutlineColor(sel ? sf::Color(accent.r,accent.g,accent.b,220)
                                     : sf::Color(70,72,82,140));
            window.draw(card);

            if (loaded) {
                sf::Sprite sp(tex);
                sp.setTextureRect(sf::IntRect(16, 0, 16, 24));
                sp.setScale(5.5f, 5.5f);
                sp.setPosition(cx - sp.getGlobalBounds().width*0.5f, cY + 22.f);
                if (!sel) sp.setColor(sf::Color(140,140,140,160));
                window.draw(sp);
            }

            sf::Text nm;
            nm.setFont(m_cinzel);
            nm.setCharacterSize(20);
            nm.setLetterSpacing(1.8f);
            std::string ns = name;
            nm.setString(sf::String::fromUtf8(ns.begin(), ns.end()));
            nm.setFillColor(sel ? sf::Color(accent.r,accent.g,accent.b,255)
                                : sf::Color(114,115,125,190));
            nm.setPosition(cx - nm.getGlobalBounds().width*0.5f, cY + cH - 55.f);
            window.draw(nm);

            sf::Text rl;
            rl.setFont(font);
            rl.setCharacterSize(14);
            rl.setStyle(sf::Text::Italic);
            std::string rs = role;
            rl.setString(sf::String::fromUtf8(rs.begin(), rs.end()));
            rl.setFillColor(sel ? sf::Color(accent.r,accent.g,accent.b,180)
                                : sf::Color(90,92,100,150));
            rl.setPosition(cx - rl.getGlobalBounds().width*0.5f, cY + cH - 28.f);
            window.draw(rl);
        };

        drawCard(250.f, 0, sf::Color(120,160,230), texM,
                 "JO\xc3\x83O", "Investigador");
        drawCard(550.f, 1, sf::Color(230,140,170), texF,
                 "RADLA", "Investigadora");

        sf::Text hint;
        hint.setFont(font);
        hint.setCharacterSize(13);
        hint.setFillColor(sf::Color(93,96,105,190));
        std::string hs = "\xe2\x86\x90 \xe2\x86\x92  escolher    \xc2\xb7    ENTER  confirmar";
        hint.setString(sf::String::fromUtf8(hs.begin(), hs.end()));
        hint.setPosition(400.f - hint.getGlobalBounds().width*0.5f, 448.f);
        window.draw(hint);
    }

    // --- Phase 8: character confirmation ---
    if (m_introPhase == 8) {
        sf::Texture texC;
        const char* spritePath = (selectedSkin == 1)
            ? "assets/maps/sprites/player/player_f.png"
            : "assets/maps/sprites/player/player_m.png";
        if (texC.loadFromFile(spritePath)) {
            sf::Sprite sp(texC);
            sp.setTextureRect(sf::IntRect(16, 0, 16, 24));
            sp.setScale(6.5f, 6.5f);
            sp.setPosition(400.f - sp.getGlobalBounds().width*0.5f, 68.f);
            window.draw(sp);
        }

        sf::Text nameT;
        nameT.setFont(m_cinzel);
        nameT.setCharacterSize(26);
        nameT.setLetterSpacing(2.f);
        std::string ns = (selectedSkin == 1) ? "Radla" : "Jo\xc3\xa3o";
        nameT.setString(sf::String::fromUtf8(ns.begin(), ns.end()));
        nameT.setFillColor(sf::Color(196,199,207,230));
        nameT.setPosition(400.f - nameT.getGlobalBounds().width*0.5f, 245.f);
        window.draw(nameT);

        sf::Text desc;
        desc.setFont(font);
        desc.setCharacterSize(16);
        desc.setStyle(sf::Text::Italic);
        desc.setFillColor(sf::Color(115,118,128,200));
        std::string ds = "A mans\xc3\xa3o te espera. Re\xc3\xba" "na os di\xc3\xa1rios, encontre a chave"
                         " \xe2\x80\x94 e escape antes que ela perceba voc\xc3\xaa.";
        desc.setString(sf::String::fromUtf8(ds.begin(), ds.end()));
        desc.setPosition(400.f - desc.getGlobalBounds().width*0.5f, 298.f);
        window.draw(desc);

        float hp = 0.5f + 0.5f * std::sin(flickerTimer * 1.8f);
        sf::Text start;
        start.setFont(m_cinzel);
        start.setCharacterSize(18);
        start.setLetterSpacing(2.f);
        start.setFillColor(sf::Color(180,183,192, sf::Uint8(160 + 95*hp)));
        std::string ss = "\xe2\x96\xb8 COME\xc3" "\x87" "AR";
        start.setString(sf::String::fromUtf8(ss.begin(), ss.end()));
        start.setPosition(400.f - start.getGlobalBounds().width*0.5f, 358.f);
        window.draw(start);
    }

    // --- Fade overlay ---
    if (m_introOverlay > 0.f) {
        sf::RectangleShape overlay(sf::Vector2f(W, H));
        overlay.setFillColor(sf::Color(0, 0, 0, static_cast<sf::Uint8>(m_introOverlay * 255.f)));
        window.draw(overlay);
    }

    drawVignette(sf::Color::Black);
}

// ── Popup de objetivo estilo RPG ──────────────────────────────────────────────
void Game::showObjPopup(const sf::String& header, const sf::String& name,
                         const sf::String& status, sf::Color accent,
                         const sf::Texture* tex, sf::IntRect rect) {
    m_objPopup = { header, name, status, accent, tex, rect, 0.f, 650.f };
    m_objPopupActive = true;
}

void Game::updateObjPopup(float dt) {
    if (!m_objPopupActive) return;
    // Congela enquanto o leitor de página estiver aberto; reinicia ao fechar
    if (pageReader.isOpen()) {
        m_objPopup.timer  = 0.f;
        m_objPopup.slideY = 650.f;
        return;
    }
    m_objPopup.timer += dt;
    const float SLIDE = 0.40f;
    if (m_objPopup.timer < SLIDE) {
        float t    = m_objPopup.timer / SLIDE;
        float ease = 1.f - (1.f - t) * (1.f - t);
        m_objPopup.slideY = 650.f + (510.f - 650.f) * ease;
    } else {
        m_objPopup.slideY = 510.f;
    }
    if (m_objPopup.timer >= 4.0f) m_objPopupActive = false;
}

void Game::drawObjPopup() {
    if (!m_objPopupActive) return;

    float alpha = (m_objPopup.timer > 3.5f)
                  ? 1.f - (m_objPopup.timer - 3.5f) / 0.5f
                  : 1.f;
    alpha = std::max(0.f, std::min(1.f, alpha));
    sf::Uint8 a = static_cast<sf::Uint8>(255.f * alpha);

    const float CW = 320.f, CH = 72.f;
    const float CX = 400.f - CW * 0.5f;
    const float CY = m_objPopup.slideY;
    const float AB =  5.f;   // accent bar width
    const float IS = 32.f;   // icon size
    const float PD = 10.f;   // padding

    sf::View prev = window.getView();
    window.setView(window.getDefaultView());

    sf::RectangleShape shadow({CW, CH});
    shadow.setPosition(CX + 3.f, CY + 3.f);
    shadow.setFillColor(sf::Color(0, 0, 0, static_cast<sf::Uint8>(130 * alpha)));
    window.draw(shadow);

    sf::RectangleShape bg({CW, CH});
    bg.setPosition(CX, CY);
    bg.setFillColor(sf::Color(10, 12, 18, static_cast<sf::Uint8>(225 * alpha)));
    window.draw(bg);

    sf::Color ac = m_objPopup.accent; ac.a = a;
    sf::RectangleShape bar({AB, CH});
    bar.setPosition(CX, CY);
    bar.setFillColor(ac);
    window.draw(bar);

    if (m_objPopup.iconTex) {
        sf::Sprite icon(*m_objPopup.iconTex);
        icon.setTextureRect(m_objPopup.iconRect);
        float sc = IS / static_cast<float>(std::max(m_objPopup.iconRect.width,
                                                     m_objPopup.iconRect.height));
        icon.setScale(sc, sc);
        icon.setPosition(CX + AB + PD, CY + (CH - IS) * 0.5f);
        icon.setColor(sf::Color(255, 255, 255, a));
        window.draw(icon);
    }

    float tx = CX + AB + PD + IS + PD;

    sf::Text hdr;
    hdr.setFont(m_cinzel);
    hdr.setCharacterSize(11);
    hdr.setLetterSpacing(1.5f);
    hdr.setString(m_objPopup.header);
    sf::Color hc = m_objPopup.accent; hc.a = a;
    hdr.setFillColor(hc);
    hdr.setPosition(tx, CY + 9.f);
    window.draw(hdr);

    sf::Text nm;
    nm.setFont(font);
    nm.setCharacterSize(13);
    nm.setString(m_objPopup.name);
    nm.setFillColor(sf::Color(215, 215, 225, a));
    nm.setPosition(tx, CY + 27.f);
    window.draw(nm);

    sf::Text st;
    st.setFont(font);
    st.setCharacterSize(11);
    st.setString(m_objPopup.status);
    st.setFillColor(sf::Color(125, 130, 148, a));
    st.setPosition(tx, CY + 48.f);
    window.draw(st);

    window.setView(prev);
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
    drawObjPopup();
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
        { std::string s = "Todo o progresso atual ser\xc3\xa1 perdido.";
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