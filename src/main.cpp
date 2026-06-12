#include <SFML/Graphics.hpp>
#include "Player.hpp"
#include "Map.hpp"

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Hollow");
    sf::Clock clock;

    Map map;
    map.load("assets/test.csv", "assets/tileset_test.png");

    Player player(100.f, 100.f);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event))
            if (event.type == sf::Event::Closed)
                window.close();

        float dt = clock.restart().asSeconds();
        player.update(dt, map);

        window.clear(sf::Color::Black);
        map.draw(window);
        player.draw(window);
        window.display();
    }

    return 0;
}
