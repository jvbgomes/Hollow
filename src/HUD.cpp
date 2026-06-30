#include "HUD.hpp"
#include <iomanip>
#include <sstream>
#include <algorithm>

// ── Pixel font 3×5 (0-9, ':', '/', ' ') ─────────────────────────────────────
static const int FW = 3, FH = 5;
static const int FONT[13][FH][FW] = {
/*0*/ {{1,1,1},{1,0,1},{1,0,1},{1,0,1},{1,1,1}},
/*1*/ {{0,1,0},{1,1,0},{0,1,0},{0,1,0},{1,1,1}},
/*2*/ {{1,1,1},{0,0,1},{0,1,1},{1,0,0},{1,1,1}},
/*3*/ {{1,1,1},{0,0,1},{0,1,1},{0,0,1},{1,1,1}},
/*4*/ {{1,0,1},{1,0,1},{1,1,1},{0,0,1},{0,0,1}},
/*5*/ {{1,1,1},{1,0,0},{1,1,1},{0,0,1},{1,1,1}},
/*6*/ {{1,1,1},{1,0,0},{1,1,1},{1,0,1},{1,1,1}},
/*7*/ {{1,1,1},{0,0,1},{0,1,0},{0,1,0},{0,1,0}},
/*8*/ {{1,1,1},{1,0,1},{1,1,1},{1,0,1},{1,1,1}},
/*9*/ {{1,1,1},{1,0,1},{1,1,1},{0,0,1},{1,1,1}},
/*:*/ {{0,0,0},{0,1,0},{0,0,0},{0,1,0},{0,0,0}},
/*/*/ {{0,0,1},{0,1,0},{0,1,0},{1,0,0},{1,0,0}},
/* */ {{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}},
};

static int charIdx(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c == ':') return 10;
    if (c == '/') return 11;
    return 12;
}

static void addPixel(sf::VertexArray& va, float x, float y, float s, sf::Color c) {
    va.append({{x,   y  }, c});
    va.append({{x+s, y  }, c});
    va.append({{x+s, y+s}, c});
    va.append({{x,   y+s}, c});
}

void HUD::drawPixelText(sf::VertexArray& va, const std::string& str,
                        float x, float y, float px, sf::Color color) {
    va.clear();
    va.setPrimitiveType(sf::Quads);
    float cx = x;
    for (char c : str) {
        int idx = charIdx(c);
        for (int r = 0; r < FH; ++r)
            for (int col = 0; col < FW; ++col)
                if (FONT[idx][r][col])
                    addPixel(va, cx + col*px, y + r*px, px, color);
        cx += (FW + 1) * px;
    }
}

// ── Padrão coração 6×7 ───────────────────────────────────────────────────────
static const int HR=6, HC=7;
static const int HEART_PAT[HR][HC] = {
    {0,1,1,0,1,1,0},
    {1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1},
    {0,1,1,1,1,1,0},
    {0,0,1,1,1,0,0},
    {0,0,0,1,0,0,0},
};

static const float PX  = 3.f;
static const float GAP = 27.f;
static const float HX  = 10.f;
static const float HY  = 8.f;

void HUD::rebuildHearts(int hp) {
    m_heartVA.clear();
    m_heartVA.setPrimitiveType(sf::Quads);
    for (int h = 0; h < 3; ++h) {
        sf::Color c = (h < hp) ? sf::Color(220,35,35) : sf::Color(55,12,12);
        float ox = HX + h * GAP;
        for (int r = 0; r < HR; ++r)
            for (int col = 0; col < HC; ++col)
                if (HEART_PAT[r][col])
                    addPixel(m_heartVA, ox + col*PX, HY + r*PX, PX, c);
    }
    m_lastHp = hp;
}

// ── Construtor ───────────────────────────────────────────────────────────────
HUD::HUD() {
    rebuildHearts(3);

    // Stamina (abaixo dos corações)
    float sw = HC * PX * 3 + GAP * 2;
    m_staminaBg.setSize({sw, 4.f});
    m_staminaBg.setPosition(HX, HY + HR * PX + 4.f);
    m_staminaFill.setSize({sw, 4.f});
    m_staminaFill.setPosition(m_staminaBg.getPosition());

    // Sprites
    const float SCALE = 0.5f;
    if (m_pageTex.loadFromFile("assets/maps/sprites/items_page.png")) {
        m_pageSprite.setTexture(m_pageTex);
        m_pageSprite.setScale(SCALE, SCALE);
        m_pageSprite.setPosition(697.f, 4.f);
    }
    if (m_lampTex.loadFromFile("assets/maps/sprites/items_candle.png")) {
        m_lampSprite.setTexture(m_lampTex);
        m_lampSprite.setScale(SCALE, SCALE);
        m_lampSprite.setPosition(697.f, 23.f);
    }
    if (m_healTex.loadFromFile("assets/maps/sprites/items_heal.png")) {
        m_healSprite.setTexture(m_healTex);
        m_healSprite.setScale(SCALE, SCALE);
        m_healSprite.setPosition(697.f, 42.f);
    }

    // Pixel-text: timer, páginas, lamparinas, poções (serão construídos no primeiro update)
    m_timerVA.setPrimitiveType(sf::Quads);
    m_pagesVA.setPrimitiveType(sf::Quads);
    m_lampsVA.setPrimitiveType(sf::Quads);
    m_potionsVA.setPrimitiveType(sf::Quads);
}

// ── Update ───────────────────────────────────────────────────────────────────
void HUD::update(int hp, int /*maxHp*/, float stamina, float maxStamina,
                  int pages, int totalPages, int lamps, float elapsed, int potions) {

    if (hp != m_lastHp) rebuildHearts(hp);

    // Stamina
    float ratio = (maxStamina > 0.f) ? std::max(0.f, stamina / maxStamina) : 1.f;
    m_staminaFill.setSize({m_staminaBg.getSize().x * ratio, 4.f});

    sf::Color fc;
    if (ratio > 0.5f)      fc = sf::Color(50, 190, 80);
    else if (ratio > 0.2f) fc = sf::Color(200, 160, 0);
    else                   fc = sf::Color(200, 50, 20);
    float ta = (ratio >= 0.99f) ? 0.f : 255.f;
    m_staminaAlpha += (ta - m_staminaAlpha) * 0.12f;
    sf::Uint8 a = static_cast<sf::Uint8>(m_staminaAlpha);
    fc.a = a;
    m_staminaFill.setFillColor(fc);
    m_staminaBg.setFillColor(sf::Color(25, 25, 25, static_cast<sf::Uint8>(a * 0.55f)));

    // Timer pixel-text
    int min = (int)elapsed / 60, sec = (int)elapsed % 60;
    std::ostringstream ss;
    ss << std::setw(2) << std::setfill('0') << min << ":"
       << std::setw(2) << std::setfill('0') << sec;
    std::string timer = ss.str();
    if (timer != m_lastTimer) {
        // Centralizado no topo: "00:00" = 5 chars × (3+1)*2 = 40px, centro em x=400
        drawPixelText(m_timerVA, timer, 380.f, 10.f, 2.f, sf::Color(140, 120, 105));
        m_lastTimer = timer;
    }

    // Páginas pixel-text
    std::string pg = std::to_string(pages) + "/" + std::to_string(totalPages);
    if (pg != m_lastPages) {
        drawPixelText(m_pagesVA, pg, 717.f, 8.f, 2.f, sf::Color(170, 160, 125));
        m_lastPages = pg;
    }

    // Lamparinas pixel-text
    std::string lp = std::to_string(lamps);
    if (lp != m_lastLamps) {
        drawPixelText(m_lampsVA, lp, 717.f, 27.f, 2.f, sf::Color(200, 140, 60));
        m_lastLamps = lp;
    }

    // Poções pixel-text
    std::string pt = std::to_string(potions);
    if (pt != m_lastPotions) {
        drawPixelText(m_potionsVA, pt, 717.f, 46.f, 2.f, sf::Color(100, 200, 130));
        m_lastPotions = pt;
    }
}

// ── Draw ─────────────────────────────────────────────────────────────────────
void HUD::draw(sf::RenderWindow& window) {
    window.draw(m_heartVA);
    window.draw(m_staminaBg);
    window.draw(m_staminaFill);
    window.draw(m_timerVA);
    window.draw(m_pageSprite);
    window.draw(m_lampSprite);
    window.draw(m_healSprite);
    window.draw(m_pagesVA);
    window.draw(m_lampsVA);
    window.draw(m_potionsVA);
}
