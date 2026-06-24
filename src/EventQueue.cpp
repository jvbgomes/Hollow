#include "EventQueue.hpp"

EventQueue::EventQueue() 
    : head(nullptr), tail(nullptr), displayTimer(0.0f), displayDuration(2.5f), hasActiveMessage(false) {
    
    textDisplay.setCharacterSize(16);
    textDisplay.setFillColor(sf::Color::Yellow);
    textDisplay.setStyle(sf::Text::Bold);
    textDisplay.setPosition(0.f, 50.f); 
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
    if (!hasActiveMessage && !isEmpty()) {
        textDisplay.setString(head->message);
        
        float textWidth = textDisplay.getGlobalBounds().width;
        textDisplay.setPosition(760.f - textWidth, 50.f);

        displayTimer = 0.0f;
        hasActiveMessage = true;
    }

    if (hasActiveMessage) {
        displayTimer += deltaTime;
        if (displayTimer >= displayDuration) {
            dequeue(); 
            hasActiveMessage = false; 
        }
    }
}

void EventQueue::draw(sf::RenderWindow& window) {
    // Só desenha se tiver uma notificação ativa na hora
    if (hasActiveMessage) {
        window.draw(textDisplay);
    }
}