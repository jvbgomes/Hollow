#include "Item.hpp"

// Inicialização do objeto, definição de texturas e posicionamento inicial
Item::Item(ItemType type, sf::Vector2f position, const sf::Texture& texture,
           sf::IntRect textureRect, float lifetime,
           std::string loreTitle, std::string loreBody)
    : type(type), texture(texture),
      m_loreTitle(std::move(loreTitle)), m_loreBody(std::move(loreBody)),
      expirationTime(lifetime), elapsedTime(0.f), isVisible(true), expired(false), next(nullptr) {
    sprite.setTexture(this->texture);
    sprite.setTextureRect(textureRect);
    sprite.setPosition(position);
}

// Atualização do temporizador interno e cálculo do efeito piscar
void Item::update(float deltaTime) {
    if (type == ItemType::Key) {
        return;
    }
    elapsedTime += deltaTime;
    
    if (elapsedTime >= expirationTime) {
        expired = true;
    }
    else if (expirationTime - elapsedTime < 4.0f) {
        isVisible = (static_cast<int>(elapsedTime * 5) % 2 == 0);
    }
}
// Controle de renderização condicional na janela do jogo
void Item::draw(sf::RenderWindow& window) {
    if (isVisible && !expired) {
        window.draw(sprite);
    }
}

// Implementação dos Getters e Setters de estado do Item
ItemType      Item::getType()       const { return type; }
sf::FloatRect Item::getBounds()     const { return sprite.getGlobalBounds(); }
bool          Item::isExpired()     const { return expired; }
Item*         Item::getNext()       const { return next; }
void          Item::setNext(Item* n)      { next = n; }
const std::string& Item::getLoreTitle() const { return m_loreTitle; }
const std::string& Item::getLoreBody()  const { return m_loreBody; }