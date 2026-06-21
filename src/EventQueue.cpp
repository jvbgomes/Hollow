#include "EventQueue.hpp"
#include <iostream>

EventQueue::EventQueue() 
    : head(nullptr), tail(nullptr), displayTimer(0.0f), displayDuration(2.5f), hasActiveMessage(false) {
    
    // Configuração visual do texto de notificação (no topo da tela e centralizado)
    textDisplay.setCharacterSize(16);
    textDisplay.setFillColor(sf::Color::Yellow);
    textDisplay.setStyle(sf::Text::Bold);
    textDisplay.setPosition(300.f, 20.f); 
}

EventQueue::~EventQueue() {
    // Limpa a memória se o jogo fechar e ainda sobrarem elementos na fila
    while (!isEmpty()) {
        dequeue();
    }
}

// Insere um elemento no fim da fila
void EventQueue::enqueue(const std::string& message) {
    EventNode* newNode = new EventNode(message);
    
    if (isEmpty()) {
        head = newNode;
        tail = newNode;
    } else {
        tail->next = newNode;
        tail = newNode;
    }
}

// Remove o elemento do inicio da fila
void EventQueue::dequeue() {
    if (isEmpty()) return;

    EventNode* temp = head;
    head = head->next;

    if (head == nullptr) {
        tail = nullptr;
    }

    delete temp;
}

bool EventQueue::isEmpty() const {
    return head == nullptr;
}

void EventQueue::loadFont(const std::string& fontPath) {
    if (font.loadFromFile(fontPath)) {
        textDisplay.setFont(font);
    }
}

void EventQueue::update(float deltaTime) {
    // Se não tem mensagem ativa na tela, mas a fila tem notificações guardadas, puxa a próxima
    if (!hasActiveMessage && !isEmpty()) {
        textDisplay.setString(head->message);
        displayTimer = 0.0f;
        hasActiveMessage = true;
    }

    // Se tem uma mensagem aparecendo, conta o tempo dela
    if (hasActiveMessage) {
        displayTimer += deltaTime;
        if (displayTimer >= displayDuration) {
            dequeue(); // Tira essa mensagem da fila
            hasActiveMessage = false; // Abre espaço para a próxima no próximo frame
        }
    }
}

void EventQueue::draw(sf::RenderWindow& window) {
    // Só desenha se tiver uma notificação ativa na hora
    if (hasActiveMessage) {
        window.draw(textDisplay);
    }
}