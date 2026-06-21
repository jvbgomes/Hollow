#include "DialogueBox.hpp"

DialogueBox::DialogueBox() 
    : textSpeed(0.04f), textTimer(0.0f), currentCharIndex(0), active(false), textFinished(false) {
    
    // Configura o fundo da caixa de texto (Fica na parte inferior da tela: 800x600)
    boxBackground.setSize(sf::Vector2f(760.f, 150.f));
    boxBackground.setFillColor(sf::Color(0, 0, 0, 220)); // Preto com transparência
    boxBackground.setOutlineColor(sf::Color::White);
    boxBackground.setOutlineThickness(2.f);
    boxBackground.setPosition(20.f, 430.f);

    // Configura a caixa do Retrato do NPC (Lado esquerdo da caixa)
    portraitBox.setSize(sf::Vector2f(120.f, 120.f));
    portraitBox.setFillColor(sf::Color::White);
    portraitBox.setOutlineColor(sf::Color::White);
    portraitBox.setOutlineThickness(1.f);
    portraitBox.setPosition(35.f, 445.f);

    // Configurações iniciais do texto de Nome
    nameText.setCharacterSize(20);
    nameText.setFillColor(sf::Color::Cyan);
    nameText.setStyle(sf::Text::Bold);
    nameText.setPosition(170.f, 440.f);

    // Configurações iniciais do texto do Diálogo
    dialogueText.setCharacterSize(18);
    dialogueText.setFillColor(sf::Color::White);
    dialogueText.setPosition(170.f, 475.f);
}

void DialogueBox::loadFont(const std::string& fontPath) {
    // Carrega a fonte para os textos da interface
    if (font.loadFromFile(fontPath)) {
        nameText.setFont(font);
        dialogueText.setFont(font);
    }
}

void DialogueBox::startDialogue(const std::string& npcName, const std::string& message, const sf::Texture& npcTexture, sf::IntRect portraitRect) {
    nameText.setString(npcName);
    fullMessage = message;
    currentDisplayedMessage = "";
    currentCharIndex = 0;
    textTimer = 0.0f;
    active = true;
    textFinished = false;

    // Aplica o recorte da textura do NPC no portrait da caixa
    portraitBox.setTexture(&npcTexture);
    portraitBox.setTextureRect(portraitRect);
}

void DialogueBox::update(float deltaTime) {
    if (!active || textFinished) return;

    textTimer += deltaTime;

    // Efeito Typewriter: Adiciona uma letra por vez baseado no textSpeed
    if (textTimer >= textSpeed) {
        textTimer = 0.0f;
        if (currentCharIndex < fullMessage.length()) {
            currentDisplayedMessage += fullMessage[currentCharIndex];
            dialogueText.setString(currentDisplayedMessage);
            currentCharIndex++;
        } else {
            textFinished = true;
        }
    }
}

void DialogueBox::advanceDialogue() {
    if (!active) return;

    if (!textFinished) {
        // Se o jogador apertar E enquanto o texto está escrevendo, pula carregando o texto todo
        textFinished = true;
        currentDisplayedMessage = fullMessage;
        dialogueText.setString(fullMessage);
    } else {
        // Se o texto já terminou, fecha a caixa (ou abre espaço para a próxima frase na main)
        active = false;
    }
}

void DialogueBox::draw(sf::RenderWindow& window) {
    if (!active) return;

    window.draw(boxBackground);
    window.draw(portraitBox);
    window.draw(nameText);
    window.draw(dialogueText);
}