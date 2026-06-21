#pragma once

#include <SFML/Graphics.hpp>
#include <string>

// Nó para a ll
struct EventNode {
    std::string message;
    EventNode* next;

    EventNode(const std::string& msg) : message(msg), next(nullptr) {}
};

class EventQueue {
private:
    EventNode* head; // Frente da fila (quem sai primeiro)
    EventNode* tail; // Fim da fila (quem entra por último)

    sf::Text textDisplay;
    sf::Font font;
    
    float displayTimer;     // Temporizador para a mensagem atual
    float displayDuration;  // Quanto tempo cada mensagem fica na tela
    bool hasActiveMessage;

public:
    EventQueue();
    ~EventQueue();

    void enqueue(const std::string& message); // Insere no fim
    void dequeue();                            // Remove do início
    bool isEmpty() const;

    void loadFont(const std::string& fontPath);
    void update(float deltaTime);
    void draw(sf::RenderWindow& window);
};