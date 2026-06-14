#pragma once

#include <SFML/Graphics.hpp>

// Definição dos tipos de itens tratáveis no jogo
enum class ItemType {
    Page,
    Lamp,
    Key
};

class Item {
private:
    // Componentes visuais e texturização do elemento no mapa
    sf::Sprite sprite;
    sf::Texture texture;
    ItemType type;
    
    // Atributos de controle do tempo de vida e efeito de piscar
    float expirationTime; 
    float elapsedTime;
    bool isVisible;
    bool expired;

    // Ponteiro de encadeamento para a estrutura de dados manual
    Item* next;

public:
    // Construtor e gerenciadores de ciclo de vida
    Item(ItemType type, sf::Vector2f position, const sf::Texture& texture, sf::IntRect textureRect, float lifetime = 15.0f);
    ~Item() = default;

    // Métodos de atualização lógica e renderização do frame
    void update(float deltaTime);
    void draw(sf::RenderWindow& window);

    // Métodos de acesso (Getters) e modificadores (Setters)
    ItemType getType() const;
    sf::FloatRect getBounds() const;
    bool isExpired() const;
    Item* getNext() const;
    void setNext(Item* nextItem);
};