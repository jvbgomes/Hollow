#pragma once

#include <SFML/Graphics.hpp>
#include <string>

enum class ItemType { Page, Lamp, Key, Heal };

class Item {
public:
    Item(ItemType type, sf::Vector2f position, const sf::Texture& texture,
         sf::IntRect textureRect, float lifetime = 15.f,
         std::string loreTitle = "", std::string loreBody = "");
    ~Item() = default;

    void update(float deltaTime);
    void draw(sf::RenderWindow& window);

    ItemType      getType()       const;
    sf::FloatRect getBounds()     const;
    bool          isExpired()     const;
    Item*         getNext()       const;
    void          setNext(Item* n);
    const std::string& getLoreTitle() const;
    const std::string& getLoreBody()  const;

private:
    sf::Sprite  sprite;
    sf::Texture texture;
    ItemType    type;
    std::string m_loreTitle;
    std::string m_loreBody;

    float expirationTime;
    float elapsedTime;
    bool  isVisible;
    bool  expired;
    Item* next;
};