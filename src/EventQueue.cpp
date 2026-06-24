#include "EventQueue.hpp"
#include <algorithm>

const float EventQueue::NOTIF_DURATION = 4.0f;
const float EventQueue::FADE_IN_TIME   = 0.25f;
const float EventQueue::FADE_OUT_TIME  = 0.6f;

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

void EventQueue::update(float dt) {
    // Avança timers e remove notificações expiradas (compacta array)
    int write = 0;
    for (int i = 0; i < m_count; ++i) {
        m_active[i].timer += dt;
        if (m_active[i].timer < NOTIF_DURATION)
            m_active[write++] = m_active[i];
    }
    m_count = write;

    // Puxa da fila enquanto houver slot livre
    while (m_count < MAX_ACTIVE && !isEmpty()) {
        const std::string& msg = head->message;
        m_active[m_count].text  = sf::String::fromUtf8(msg.begin(), msg.end());
        m_active[m_count].timer = 0.f;
        ++m_count;
        dequeue();
    }
}

void EventQueue::draw(sf::RenderWindow& window) {
    if (m_count == 0) return;

    const float SLOT_H  = 26.f;
    const float BASE_Y  = 568.f;   // linha mais baixa
    const float RIGHT_X = 790.f;

    sf::Text text;
    text.setFont(m_font);
    text.setCharacterSize(15);
    text.setStyle(sf::Text::Bold);

    for (int i = 0; i < m_count; ++i) {
        float t = m_active[i].timer;

        // Alpha: fade-in nos primeiros FADE_IN_TIME, pleno, fade-out nos últimos FADE_OUT_TIME
        float alpha;
        if (t < FADE_IN_TIME)
            alpha = 255.f * (t / FADE_IN_TIME);
        else if (t > NOTIF_DURATION - FADE_OUT_TIME)
            alpha = 255.f * (NOTIF_DURATION - t) / FADE_OUT_TIME;
        else
            alpha = 255.f;

        sf::Uint8 a = static_cast<sf::Uint8>(std::max(0.f, std::min(255.f, alpha)));

        text.setString(m_active[i].text);
        text.setFillColor(sf::Color(255, 230, 100, a));

        float textW = text.getGlobalBounds().width;
        // Notificação mais nova (índice 0) fica mais baixo; mais antigas sobem
        float y = BASE_Y - i * SLOT_H;
        text.setPosition(RIGHT_X - textW, y);

        // Fundo semi-transparente para legibilidade
        sf::RectangleShape bg(sf::Vector2f(textW + 12.f, 20.f));
        bg.setFillColor(sf::Color(0, 0, 0, static_cast<sf::Uint8>(a * 0.55f)));
        bg.setPosition(RIGHT_X - textW - 6.f, y + 1.f);
        window.draw(bg);
        window.draw(text);
    }
}
