#include "ItemList.hpp"

// Construtor e Destrutor com limpeza de memória garantida
ItemList::ItemList() : head(nullptr) {}

ItemList::~ItemList() {
    clear();
}

// Inserção de novos elementos no início da lista encadeada (O(1))
void ItemList::addItem(ItemType type, sf::Vector2f position, const sf::Texture& texture,
                        sf::IntRect textureRect, float lifetime,
                        const std::string& loreTitle, const std::string& loreBody) {
    Item* newItem = new Item(type, position, texture, textureRect, lifetime, loreTitle, loreBody);
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

bool ItemList::getNearbyItemInfo(const sf::FloatRect& playerBounds,
                                  sf::Vector2f& outPos, ItemType& outType) const {
    Item* current = head;
    while (current != nullptr) {
        if (current->getBounds().intersects(playerBounds)) {
            sf::FloatRect b = current->getBounds();
            outPos  = {b.left + b.width / 2.f, b.top + b.height / 2.f};
            outType = current->getType();
            return true;
        }
        current = current->getNext();
    }
    return false;
}

// Varredura sequencial para renderização de todos os elementos ativos
void ItemList::draw(sf::RenderWindow& window) {
    Item* current = head;
    while (current != nullptr) {
        current->draw(window);
        current = current->getNext();
    }
}

bool ItemList::hasNearbyItem(const sf::FloatRect& playerBounds) const {
    Item* current = head;
    while (current != nullptr) {
        if (current->getBounds().intersects(playerBounds)) return true;
        current = current->getNext();
    }
    return false;
}

// Varredura de colisões (Interseção de caixas) para ativação da Autocoleta
bool ItemList::checkAutoCollect(const sf::FloatRect& playerBounds, ItemType& outType,
                                 std::string& outLoreTitle, std::string& outLoreBody) {
    Item* current = head;
    while (current != nullptr) {
        if (current->getBounds().intersects(playerBounds)) {
            outType      = current->getType();
            outLoreTitle = current->getLoreTitle();
            outLoreBody  = current->getLoreBody();
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