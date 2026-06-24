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
    void addItem(ItemType type, sf::Vector2f position, const sf::Texture& texture,
                 sf::IntRect textureRect, float lifetime = 15.f,
                 const std::string& loreTitle = "", const std::string& loreBody = "");
    void removeItem(Item* itemToRemove);

    // Loops de processamento em lote (Atualização e Desenho de todos os itens)
    void update(float deltaTime);
    void draw(sf::RenderWindow& window);
    void clear();

    // Verifica se há item próximo sem coletar
    bool hasNearbyItem(const sf::FloatRect& playerBounds) const;

    // Retorna posição e tipo do item próximo (false se nenhum)
    bool getNearbyItemInfo(const sf::FloatRect& playerBounds,
                           sf::Vector2f& outPos, ItemType& outType) const;

    // Coleta o item próximo (requer que hasNearbyItem retorne true)
    bool checkAutoCollect(const sf::FloatRect& playerBounds, ItemType& outType,
                          std::string& outLoreTitle, std::string& outLoreBody);
};