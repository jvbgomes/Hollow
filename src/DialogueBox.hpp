#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

class DialogueBox {
private:
    sf::RectangleShape boxBackground;
    sf::RectangleShape portraitBox;
    sf::Text nameText;
    sf::Text dialogueText;
    sf::Font font;

    std::string fullMessage;
    std::string currentDisplayedMessage;
    
    float textSpeed; // Tempo entre cada letra
    float textTimer;
    size_t currentCharIndex;
    
    bool active;
    bool textFinished;

public:
    DialogueBox();
    ~DialogueBox() = default;

    void loadFont(const std::string& fontPath);
    void startDialogue(const std::string& npcName, const std::string& message, const sf::Texture& npcTexture, sf::IntRect portraitRect);
    
    void update(float deltaTime);
    void draw(sf::RenderWindow& window);
    
    void advanceDialogue();

    bool isActive() const { return active; }
    bool isTextFinished() const { return textFinished; }
    void close() { active = false; }
};