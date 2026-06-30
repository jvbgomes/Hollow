#pragma once

#include <SFML/Graphics.hpp>
#include <string>

struct EventNode {
    std::string message;
    EventNode*  next;
    EventNode(const std::string& msg) : message(msg), next(nullptr) {}
};

class EventQueue {
public:
    EventQueue();
    ~EventQueue();

    void enqueue(const std::string& message);
    void dequeue();
    bool isEmpty() const;

    void loadFont(const std::string& fontPath);
    void update(float deltaTime);
    void draw(sf::RenderWindow& window);

private:
    EventNode* head;
    EventNode* tail;

    sf::Font m_font;

    static const int   MAX_ACTIVE = 4;
    static const float NOTIF_DURATION;
    static const float SLIDE_TIME;
    static const float FADE_OUT_TIME;

    struct ActiveNotif {
        sf::String  text;
        float       timer    = 0.f;
        float       targetY  = 0.f;   // posição Y de destino
        float       currentY = 0.f;   // posição Y atual (interpolada)
    };

    ActiveNotif m_active[MAX_ACTIVE];
    int         m_count = 0;

    // Devolve cor de destaque baseada no conteúdo da mensagem
    static sf::Color accentColor(const sf::String& text);
};
