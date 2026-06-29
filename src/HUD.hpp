#pragma once
#include <SFML/Graphics.hpp>
#include <string>

class HUD {
public:
    HUD();
    void update(int hp, int maxHp, float stamina, float maxStamina,
                int pages, int totalPages, int lamps, float elapsed, int potions);
    void draw(sf::RenderWindow& window);

private:
    // Corações pixel-art
    sf::VertexArray m_heartVA;
    int m_lastHp = -1;

    // Barra de stamina fina
    sf::RectangleShape m_staminaBg;
    sf::RectangleShape m_staminaFill;
    float m_staminaAlpha = 0.f;

    // Textos pixel-art (VertexArray)
    sf::VertexArray m_timerVA;
    sf::VertexArray m_pagesVA;
    sf::VertexArray m_lampsVA;
    sf::VertexArray m_potionsVA;

    // Sprites dos itens
    sf::Texture m_pageTex;
    sf::Texture m_lampTex;
    sf::Texture m_healTex;
    sf::Sprite  m_pageSprite;
    sf::Sprite  m_lampSprite;
    sf::Sprite  m_healSprite;

    // Cache para só reconstruir quando mudar
    std::string m_lastTimer;
    std::string m_lastPages;
    std::string m_lastLamps;
    std::string m_lastPotions;

    void rebuildHearts(int hp);
    static void drawPixelText(sf::VertexArray& va, const std::string& str,
                              float x, float y, float px, sf::Color color);
};
