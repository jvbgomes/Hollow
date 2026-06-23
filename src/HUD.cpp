#include "HUD.hpp"
#include <iomanip>
#include <sstream>

HUD::HUD() {
    //  Configuração da Barra de Vida (Vermelha) 
    healthBarBack.setSize(sf::Vector2f(150.f, 15.f));
    healthBarBack.setFillColor(sf::Color(50, 0, 0)); // Vermelho escuro de fundo
    healthBarBack.setPosition(20.f, 20.f);

    healthBarFront.setFillColor(sf::Color(200, 0, 0)); // Vermelho vivo para a vida atual
    healthBarFront.setPosition(20.f, 20.f);

    //  Configuração da Barra de Stamina (Verde ou Amarela) 
    staminaBarBack.setSize(sf::Vector2f(150.f, 10.f));
    staminaBarBack.setFillColor(sf::Color(30, 30, 30));
    staminaBarBack.setPosition(20.f, 40.f);

    staminaBarFront.setFillColor(sf::Color(0, 180, 80)); // Verde para indicar fôlego
    staminaBarFront.setPosition(20.f, 40.f);

    //  Configuração dos Textos 
    pagesText.setCharacterSize(16);
    pagesText.setFillColor(sf::Color::White);
    pagesText.setPosition(250.f, 20.f);

    lampsText.setCharacterSize(16);
    lampsText.setFillColor(sf::Color::White);
    lampsText.setPosition(420.f, 20.f);

    timerText.setCharacterSize(16);
    timerText.setFillColor(sf::Color::White);
    timerText.setPosition(680.f, 20.f);
}

void HUD::loadFont(const std::string& fontPath) {
    if (font.loadFromFile(fontPath)) {
        pagesText.setFont(font);
        lampsText.setFont(font);
        timerText.setFont(font);
    }
}

void HUD::update(int currentHealth, int maxHealth, float currentStamina, float maxStamina, 
                  int collectedPages, int totalPages, int remainingLamps, float elapsedTime) {
    
    // Atualiza tamanho da barra de vida proporcionalmente
    if (maxHealth > 0) {
        float healthRatio = static_cast<float>(currentHealth) / maxHealth;
        healthBarFront.setSize(sf::Vector2f(150.f * healthRatio, 15.f));
    }

    // Atualiza tamanho da barra de stamina proporcionalmente
    if (maxStamina > 0) {
        float staminaRatio = currentStamina / maxStamina;
        staminaBarFront.setSize(sf::Vector2f(150.f * staminaRatio, 10.f));
    }

    // Atualiza os contadores de texto
    pagesText.setString("Pages: " + std::to_string(collectedPages) + " / " + std::to_string(totalPages));
    lampsText.setString("Lamps: " + std::to_string(remainingLamps));

    // Formata o cronômetro para mm:ss
    int minutes = static_cast<int>(elapsedTime) / 60;
    int seconds = static_cast<int>(elapsedTime) % 60;
    std::stringstream ss;
    ss << std::setw(2) << std::setfill('0') << minutes << ":"
       << std::setw(2) << std::setfill('0') << seconds;
    timerText.setString("Time: " + ss.str());
}

void HUD::draw(sf::RenderWindow& window) {
    // Desenha as barras
    window.draw(healthBarBack);
    window.draw(healthBarFront);
    window.draw(staminaBarBack);
    window.draw(staminaBarFront);

    // Desenha os textos
    window.draw(pagesText);
    window.draw(lampsText);
    window.draw(timerText);
}