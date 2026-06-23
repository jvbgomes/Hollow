#pragma once

#include <SFML/Graphics.hpp>
#include <string>

class HUD {
private:
    sf::Font font;
    
    // Textos do HUD
    sf::Text pagesText;
    sf::Text lampsText;
    sf::Text timerText;

    // Elementos visuais para as barras (Vida e Stamina)
    sf::RectangleShape healthBarBack;
    sf::RectangleShape healthBarFront;
    sf::RectangleShape staminaBarBack;
    sf::RectangleShape staminaBarFront;

public:
    HUD();
    void loadFont(const std::string& fontPath);
    
    // Atualiza os dados que vêm  do loop do jogo e do Player
    void update(int currentHealth, int maxHealth, float currentStamina, float maxStamina, 
                int collectedPages, int totalPages, int remainingLamps, float elapsedTime);
                
    void draw(sf::RenderWindow& window);
};