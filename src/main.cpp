#include <SFML/Graphics.hpp>
#include <iostream>
#include "Player.hpp"
#include "Map.hpp"
#include "NPC.hpp"

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Hollow");
    sf::Clock clock;

    Map map;
    map.load("assets/test.csv", "assets/tileset_test.png");

    Player player(100.f, 100.f);

    sf::Texture texturaTeste;
    texturaTeste.loadFromFile("assets/tileset_test.png");

    NPC fantasma("Gasparzinho", 300.f, 300.f, texturaTeste, sf::IntRect(0, 0, 32, 32));
    
    std::vector<std::string> dialogoOpcao0 = {
        "Ola, viajante... Quem e voce?",
        "Esta mansao esconde segredos obscuros."
    };
    fantasma.addDialogueOption(0, dialogoOpcao0);

    bool teclaPressionada = false;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        float dt = clock.restart().asSeconds();
        player.update(dt, map);

        fantasma.update(dt, map);

        if (fantasma.isPlayerNearby(player.getPosition())) {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::E)) {
                if (!teclaPressionada) {
                    if (fantasma.hasDialogue(0)) {
                        std::cout << fantasma.getName() << ": " << fantasma.getNextSentence(0) << std::endl;
                    } else {
                        std::cout << fantasma.getName() << " nao tem mais nada a dizer." << std::endl;
                    }
                    teclaPressionada = true;
                }
            } else {
                teclaPressionada = false;
            }
        }

        window.clear(sf::Color::Black);
        map.draw(window);
        
        fantasma.draw(window);
        
        player.draw(window);
        window.display();
    }

    return 0;
}