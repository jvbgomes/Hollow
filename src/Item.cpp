#include "Item.hpp"
#include <cmath>

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
    if (type == ItemType::Page)
        m_glowTimer += deltaTime;
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
    if (!isVisible || expired) return;

    if (type == ItemType::Page) {
        // Flash de borda a cada 3s: acende em 0.3s, apaga em 0.3s, depois fica off
        float cycle = std::fmod(m_glowTimer, 3.0f);
        float pulse;
        if      (cycle < 0.3f) pulse = cycle / 0.3f;
        else if (cycle < 0.6f) pulse = (0.6f - cycle) / 0.3f;
        else                   pulse = 0.f;
        sf::Uint8 borderA = static_cast<sf::Uint8>(220 * pulse);

        sf::Sprite glowSprite = sprite;
        glowSprite.setColor(sf::Color(255, 215, 30, borderA));

        sf::Vector2f pos = sprite.getPosition();
        const float D = 2.f;
        const float offsets[8][2] = {
            {0,-D},{0,D},{-D,0},{D,0},
            {-D,-D},{D,-D},{-D,D},{D,D}
        };
        for (auto& off : offsets) {
            glowSprite.setPosition(pos.x + off[0], pos.y + off[1]);
            window.draw(glowSprite);
        }
        glowSprite.setPosition(pos);
    }

    window.draw(sprite);
}

// Implementação dos Getters e Setters de estado do Item
ItemType      Item::getType()       const { return type; }
sf::FloatRect Item::getBounds()     const { return sprite.getGlobalBounds(); }
bool          Item::isExpired()     const { return expired; }
Item*         Item::getNext()       const { return next; }
void          Item::setNext(Item* n)      { next = n; }
const std::string& Item::getLoreTitle() const { return m_loreTitle; }
const std::string& Item::getLoreBody()  const { return m_loreBody; }