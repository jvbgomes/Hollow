#include "ItemList.hpp"

// Construtor e Destrutor com limpeza de memória garantida
ItemList::ItemList() : head(nullptr) {}

ItemList::~ItemList() {
    clear();
}

// Inserção de novos elementos no início da lista encadeada (O(1))
void ItemList::addItem(ItemType type, sf::Vector2f position, const sf::Texture& texture, sf::IntRect textureRect, float lifetime) {
    Item* newItem = new Item(type, position, texture, textureRect, lifetime);
    newItem->setNext(head);
    head = newItem;
}

// Lógica de busca e rearranjo de ponteiros para exclusão de um nó da memória
void ItemList::removeItem(Item* itemToRemove) {
    if (!head || !itemToRemove) return;

    if (head == itemToRemove) {
        head = head->getNext();
        delete itemToRemove;
        return;
    }

    Item* current = head;
    while (current->getNext() && current->getNext() != itemToRemove) {
        current = current->getNext();
    }

    if (current->getNext() == itemToRemove) {
        current->setNext(itemToRemove->getNext());
        delete itemToRemove;
    }
}

// Varredura sequencial para atualizar tempos e remover itens expirados
void ItemList::update(float deltaTime) {
    Item* current = head;
    while (current != nullptr) {
        Item* next = current->getNext();
        current->update(deltaTime);
        
        if (current->isExpired()) {
            removeItem(current);
        }
        current = next;
    }
}

// Varredura sequencial para renderização de todos os elementos ativos
void ItemList::draw(sf::RenderWindow& window) {
    Item* current = head;
    while (current != nullptr) {
        current->draw(window);
        current = current->getNext();
    }
}

// Varredura de colisões (Interseção de caixas) para ativação da Autocoleta
bool ItemList::checkAutoCollect(const sf::FloatRect& playerBounds, ItemType& outCollectedType) {
    Item* current = head;
    
    while (current != nullptr) {
        if (current->getBounds().intersects(playerBounds)) {
            outCollectedType = current->getType();
            removeItem(current);
            return true;
        }
        current = current->getNext();
    }
    return false;
}

// Desalocação iterativa de toda a lista para evitar vazamento de memória (Memory Leak)
void ItemList::clear() {
    Item* current = head;
    while (current != nullptr) {
        Item* next = current->getNext();
        delete current;
        current = next;
    }
    head = nullptr;
}