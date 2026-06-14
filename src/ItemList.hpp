#pragma once

#include "Item.hpp"

class ItemList {
private:
    // Ponteiro âncora para o início da estrutura encadeada
    Item* head;

public:
    // Gerenciamento de inicialização e destruição da lista
    ItemList();
    ~ItemList();

    // Operações de manipulação de nós (Inserção e Remoção manual)
    void addItem(ItemType type, sf::Vector2f position, const sf::Texture& texture, sf::IntRect textureRect, float lifetime = 15.0f);
    void removeItem(Item* itemToRemove);

    // Loops de processamento em lote (Atualização e Desenho de todos os itens)
    void update(float deltaTime);
    void draw(sf::RenderWindow& window);
    void clear();

    // Motor de detecção e execução da mecânica de Autocoleta
    bool checkAutoCollect(const sf::FloatRect& playerBounds, ItemType& outCollectedType);
};