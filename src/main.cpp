#include <SFML/Graphics.hpp>
#include <iostream>
#include "Player.hpp"
#include "Map.hpp"
#include "NPC.hpp"
#include "DialogueBox.hpp"
#include "EventQueue.hpp"

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Hollow");
    sf::Clock clock;

    Map map;
    map.load("assets/test.csv", "assets/tileset_test.png");

    Player player(100.f, 100.f);

    sf::Texture testTexture;
    testTexture.loadFromFile("assets/tileset_test.png");

    // Instancia o NPC Gasparzinho (teste)
    NPC ghost("Gasparzinho", 300.f, 300.f, testTexture, sf::IntRect(0, 0, 32, 32));
    
    std::vector<std::string> dialogueOption0 = {
        "Ola, viajante... Quem e voce?",
        "Esta mansao esconde segredos obscuros."
    };
    ghost.addDialogueOption(0, dialogueOption0);

    // Instancia e configura a interface da caixa de dialogo
    DialogueBox dialogueBox;
    dialogueBox.loadFont("assets/Arial.ttf");

    EventQueue eventQueue;
    eventQueue.loadFont("assets/Arial.ttf");

    bool keyPressed = false;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::G) {
                static int itemCounter = 1;
                eventQueue.enqueue("Item coletado de numero: " + std::to_string(itemCounter++));
            }
        }

        float dt = clock.restart().asSeconds();
        player.update(dt, map);

        ghost.update(dt, map);
        dialogueBox.update(dt);
        eventQueue.update(dt);

        if (ghost.isPlayerNearby(player.getPosition())) {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::E)) {
                if (!keyPressed) {
                    if (dialogueBox.isActive()) {
                        // Se a caixa já está ativa na tela, a tecla E avança o texto
                        dialogueBox.advanceDialogue();
                    } else {
                        if (ghost.hasDialogue(0)) {
                            // Se começou a falar, dispara a notificação do item na fila de eventos!
                            if (ghost.hasDialogue(0)) { 
                                eventQueue.enqueue("Pagina da Mansao Coletada!");
                            }
                            
                            std::string nextSentence = ghost.getNextSentence(0);
                            dialogueBox.startDialogue(ghost.getName(), nextSentence, testTexture, sf::IntRect(0, 0, 32, 32));
                        } else {
                            // Reinicia as frases para o jogador poder interagir de novo!
                            std::vector<std::string> reloadDialogue = {
                                "Ola, viajante... Quem e voce?",
                                "Esta mansao esconde segredos obscuros."
                            };
                            ghost.addDialogueOption(0, reloadDialogue);
                            
                            std::string nextSentence = ghost.getNextSentence(0);
                            dialogueBox.startDialogue(ghost.getName(), nextSentence, testTexture, sf::IntRect(0, 0, 32, 32));
                        }
                    }
                    keyPressed = true;
                }
            } else {
                keyPressed = false;
            }
        } else {
            if (dialogueBox.isActive()) {
                dialogueBox.close();
            }
        }

        window.clear(sf::Color::Black);
        map.draw(window);
        
        ghost.draw(window);
        player.draw(window);
        
        // Desenha a caixa de dialogo
        dialogueBox.draw(window);
        eventQueue.draw(window);
        
        window.display();
    }

    return 0;
}