#include "EventQueue.hpp"
#include <algorithm>
#include <cmath>

const float EventQueue::NOTIF_DURATION = 3.5f;
const float EventQueue::SLIDE_TIME     = 0.22f;
const float EventQueue::FADE_OUT_TIME  = 0.55f;

EventQueue::EventQueue() : head(nullptr), tail(nullptr), m_count(0) {}

EventQueue::~EventQueue() {
    while (!isEmpty()) dequeue();
}

void EventQueue::enqueue(const std::string& message) {
    EventNode* node = new EventNode(message);
    if (isEmpty()) { head = node; tail = node; }
    else           { tail->next = node; tail = node; }
}

void EventQueue::dequeue() {
    if (isEmpty()) return;
    EventNode* tmp = head;
    head = head->next;
    if (!head) tail = nullptr;
    delete tmp;
}

bool EventQueue::isEmpty() const { return head == nullptr; }

void EventQueue::loadFont(const std::string& path) {
    m_font.loadFromFile(path);
}

sf::Color EventQueue::accentColor(const sf::String& text) {
    std::string s;
    for (sf::Uint32 c : text) s += (c < 128) ? (char)c : '?';

    auto has = [&](const char* kw) {
        return s.find(kw) != std::string::npos;
    };

    if (has("di") || has("gina") || has("livro"))   return sf::Color(255, 210,  80);  // ouro — página
    if (has("Lamp") || has("lamp") || has("sal"))    return sf::Color( 80, 190, 255);  // azul — lamparina
    if (has("Chave") || has("chave"))                return sf::Color(120, 240, 140);  // verde — chave
    if (has("Po") || has("po") || has("ora"))        return sf::Color(230, 100, 140);  // rosa — poção/coração
    return sf::Color(200, 200, 210);
}

void EventQueue::update(float dt) {
    // Posições Y alvo: slot 0 (mais novo) na base, slots acima sobem 42px cada
    const float BASE_Y = 556.f;
    const float STEP   = 42.f;

    // Avança timers, remove expiradas
    int write = 0;
    for (int i = 0; i < m_count; ++i) {
        m_active[i].timer += dt;
        if (m_active[i].timer < NOTIF_DURATION)
            m_active[write++] = m_active[i];
    }
    m_count = write;

    // Puxa da fila para slots livres (mais novo → índice 0)
    while (m_count < MAX_ACTIVE && !isEmpty()) {
        // Empurra existentes para cima (índice maior)
        for (int i = m_count; i > 0; --i)
            m_active[i] = m_active[i-1];

        const std::string& msg = head->message;
        m_active[0].text    = sf::String::fromUtf8(msg.begin(), msg.end());
        m_active[0].timer   = 0.f;
        m_active[0].targetY = BASE_Y;
        m_active[0].currentY = BASE_Y;
        ++m_count;
        dequeue();
    }

    // Atualiza targetY e interpola currentY
    for (int i = 0; i < m_count; ++i) {
        m_active[i].targetY = BASE_Y - i * STEP;
        float diff = m_active[i].targetY - m_active[i].currentY;
        m_active[i].currentY += diff * std::min(1.f, dt * 12.f);  // lerp rápido
    }
}

void EventQueue::draw(sf::RenderWindow& window) {
    if (m_count == 0) return;

    const float RIGHT_X     = 796.f;
    const float CARD_H      = 30.f;
    const float PAD_H       = 10.f;   // padding horizontal interno
    const float ACCENT_W    =  4.f;   // barra colorida à esquerda

    sf::Text text;
    text.setFont(m_font);
    text.setCharacterSize(14);

    for (int i = 0; i < m_count; ++i) {
        float t = m_active[i].timer;

        // Alpha: slide-in no início, pleno, fade-out no fim
        float alpha;
        if (t < SLIDE_TIME)
            alpha = 255.f * (t / SLIDE_TIME);
        else if (t > NOTIF_DURATION - FADE_OUT_TIME)
            alpha = 255.f * (NOTIF_DURATION - t) / FADE_OUT_TIME;
        else
            alpha = 255.f;
        alpha = std::max(0.f, std::min(255.f, alpha));

        // Slide horizontal: durante SLIDE_TIME a notificação entra da direita
        float slideProgress = std::min(1.f, t / SLIDE_TIME);
        // ease-out: sqrt suaviza a chegada
        float ease     = 1.f - std::sqrt(1.f - slideProgress * slideProgress);
        float slideOff = (1.f - ease) * 80.f;   // desloca até 80px para a direita no início

        sf::Uint8 a  = static_cast<sf::Uint8>(alpha);
        sf::Color accent = accentColor(m_active[i].text);
        accent.a = a;

        text.setString(m_active[i].text);
        float textW = text.getGlobalBounds().width;
        float cardW = textW + PAD_H * 2.f + ACCENT_W;

        float y = m_active[i].currentY;
        float x = RIGHT_X - cardW + slideOff;

        // Sombra do card (deslocada 2px)
        sf::RectangleShape shadow({cardW, CARD_H});
        shadow.setPosition(x + 2.f, y + 2.f);
        shadow.setFillColor(sf::Color(0, 0, 0, static_cast<sf::Uint8>(alpha * 0.4f)));
        window.draw(shadow);

        // Fundo do card
        sf::RectangleShape bg({cardW, CARD_H});
        bg.setPosition(x, y);
        bg.setFillColor(sf::Color(14, 16, 22, static_cast<sf::Uint8>(alpha * 0.88f)));
        window.draw(bg);

        // Barra de cor à esquerda
        sf::RectangleShape bar({ACCENT_W, CARD_H});
        bar.setPosition(x, y);
        bar.setFillColor(accent);
        window.draw(bar);

        // Texto
        text.setFillColor(sf::Color(230, 230, 240, a));
        text.setPosition(x + ACCENT_W + PAD_H, y + CARD_H * 0.5f - text.getGlobalBounds().height * 0.5f - 2.f);
        window.draw(text);
    }
}
