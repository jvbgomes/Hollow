#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

class PageReader {
public:
    PageReader();
    void loadFont(const std::string& fontPath);
    void open(const std::string& title, const std::string& body);
    void close();
    bool isOpen() const;
    void handleInput();   // chama no update quando isOpen()
    void draw(sf::RenderWindow& window);

private:
    bool m_open = false;

    sf::Font           m_font;
    sf::RectangleShape m_overlay;
    sf::RectangleShape m_panel;
    sf::RectangleShape m_divider;
    sf::Text           m_titleText;
    sf::Text           m_bodyText;
    sf::Text           m_hintText;
    sf::Text           m_pageNumText;

    std::string              m_title;
    std::vector<std::string> m_pages;   // corpo dividido em páginas
    int                      m_curPage = 0;

    bool m_leftWasPressed  = false;
    bool m_rightWasPressed = false;

    void buildPages(const std::string& body);
    void applyPage();
};
