#include "PageReader.hpp"
#include <sstream>

static sf::String u8(const std::string& s) {
    return sf::String::fromUtf8(s.begin(), s.end());
}

static const float PW = 540.f;
static const float PH = 400.f;
static const float PX = (800.f - PW) / 2.f;
static const float PY = (600.f - PH) / 2.f;
static const float BODY_MAX_H = 285.f;   // altura máxima do corpo antes de paginar

PageReader::PageReader() {
    m_overlay.setSize({800.f, 600.f});
    m_overlay.setFillColor(sf::Color(0, 0, 0, 200));
    m_overlay.setPosition(0.f, 0.f);

    m_panel.setSize({PW, PH});
    m_panel.setFillColor(sf::Color(18, 13, 8));
    m_panel.setOutlineColor(sf::Color(130, 100, 55));
    m_panel.setOutlineThickness(3.f);
    m_panel.setPosition(PX, PY);

    m_divider.setSize({PW - 50.f, 1.f});
    m_divider.setFillColor(sf::Color(130, 100, 55, 200));
    m_divider.setPosition(PX + 25.f, PY + 62.f);

    m_titleText.setCharacterSize(19);
    m_titleText.setFillColor(sf::Color(220, 190, 120));
    m_titleText.setStyle(sf::Text::Bold | sf::Text::Italic);
    m_titleText.setPosition(PX + 25.f, PY + 20.f);

    m_bodyText.setCharacterSize(13);
    m_bodyText.setFillColor(sf::Color(195, 182, 158));
    m_bodyText.setLineSpacing(1.55f);
    m_bodyText.setPosition(PX + 25.f, PY + 76.f);

    m_hintText.setCharacterSize(11);
    m_hintText.setFillColor(sf::Color(100, 85, 60));
    m_hintText.setStyle(sf::Text::Italic);

    m_pageNumText.setCharacterSize(11);
    m_pageNumText.setFillColor(sf::Color(110, 90, 60));
}

void PageReader::loadFont(const std::string& path) {
    if (m_font.loadFromFile(path)) {
        m_titleText.setFont(m_font);
        m_bodyText.setFont(m_font);
        m_hintText.setFont(m_font);
        m_pageNumText.setFont(m_font);
    }
}

// Divide o corpo em páginas que cabem na altura BODY_MAX_H
void PageReader::buildPages(const std::string& body) {
    m_pages.clear();

    // Usa o bodyText para medir altura: testa linha por linha
    std::istringstream stream(body);
    std::string line, accumulated;

    auto measure = [&](const std::string& s) -> float {
        m_bodyText.setString(u8(s));
        return m_bodyText.getGlobalBounds().height;
    };

    while (std::getline(stream, line)) {
        std::string candidate = accumulated.empty() ? line : accumulated + "\n" + line;
        if (!accumulated.empty() && measure(candidate) > BODY_MAX_H) {
            m_pages.push_back(accumulated);
            accumulated = line;
        } else {
            accumulated = candidate;
        }
    }
    if (!accumulated.empty())
        m_pages.push_back(accumulated);

    if (m_pages.empty()) m_pages.push_back("");
}

void PageReader::applyPage() {
    m_titleText.setString(u8(m_title));
    if (m_curPage < (int)m_pages.size())
        m_bodyText.setString(u8(m_pages[m_curPage]));

    bool multi = m_pages.size() > 1;
    std::string hint = multi ? "[A] anterior   [E] fechar   [D] proxima"
                             : "[E] fechar";
    m_hintText.setString(u8(hint));
    m_hintText.setPosition(PX + (PW - m_hintText.getGlobalBounds().width) / 2.f,
                           PY + PH - 24.f);

    if (multi) {
        std::string pg = std::to_string(m_curPage + 1) + " / " +
                         std::to_string((int)m_pages.size());
        m_pageNumText.setString(pg);
        m_pageNumText.setPosition(PX + 25.f, PY + PH - 24.f);
    } else {
        m_pageNumText.setString("");
    }
}

void PageReader::open(const std::string& title, const std::string& body) {
    m_title   = title;
    m_curPage = 0;
    buildPages(body);
    applyPage();
    m_open = true;
    m_leftWasPressed  = false;
    m_rightWasPressed = false;
}

void PageReader::close() { m_open = false; }
bool PageReader::isOpen() const { return m_open; }

void PageReader::handleInput() {
    // Página seguinte — D ou seta direita
    bool rightNow = sf::Keyboard::isKeyPressed(sf::Keyboard::D) ||
                    sf::Keyboard::isKeyPressed(sf::Keyboard::Right);
    if (rightNow && !m_rightWasPressed) {
        if (m_curPage < (int)m_pages.size() - 1) {
            m_curPage++;
            applyPage();
        }
    }
    m_rightWasPressed = rightNow;

    // Página anterior — A ou seta esquerda
    bool leftNow = sf::Keyboard::isKeyPressed(sf::Keyboard::A) ||
                   sf::Keyboard::isKeyPressed(sf::Keyboard::Left);
    if (leftNow && !m_leftWasPressed) {
        if (m_curPage > 0) {
            m_curPage--;
            applyPage();
        }
    }
    m_leftWasPressed = leftNow;
}

void PageReader::draw(sf::RenderWindow& window) {
    if (!m_open) return;
    window.draw(m_overlay);
    window.draw(m_panel);
    window.draw(m_divider);
    window.draw(m_titleText);
    window.draw(m_bodyText);
    window.draw(m_hintText);
    window.draw(m_pageNumText);
}
