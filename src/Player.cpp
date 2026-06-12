#include "Player.hpp"
#include "Map.hpp"

Player::Player(float x, float y) : Entity(x, y) {
    speed = 150.f;
    stamina = 100.f;
    maxStamina = 100.f;
    isSprinting = false;
    health = 3;
    diaryPages = 0;
    saltLanterns = 0;

    // Placeholder visual até ter sprite real
    sf::Image img;
    img.create(24, 24, sf::Color(200, 100, 100));
    texture.loadFromImage(img);
    sprite.setTexture(texture);
    sprite.setOrigin(12.f, 12.f);
    sprite.setPosition(position);
}

void Player::handleInput() {
    velocity = sf::Vector2f(0.f, 0.f);

    float currentSpeed = speed;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) && stamina > 0) {
        currentSpeed = speed * 1.6f;
        isSprinting = true;
    } else {
        isSprinting = false;
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) velocity.y = -currentSpeed;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) velocity.y =  currentSpeed;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) velocity.x = -currentSpeed;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) velocity.x =  currentSpeed;
}

void Player::update(float dt, const Map& map) {
    handleInput();

    if (isSprinting && stamina > 0) {
        stamina -= 30.f * dt;
        if (stamina < 0.f) stamina = 0.f;
    } else if (!isSprinting && stamina < maxStamina) {
        stamina += 15.f * dt;
        if (stamina > maxStamina) stamina = maxStamina;
    }

    // Colisão por eixo separado — permite deslizar nas paredes
    position.x += velocity.x * dt;
    sprite.setPosition(position);
    if (map.isCollidingWith(sprite.getGlobalBounds())) {
        position.x -= velocity.x * dt;
        sprite.setPosition(position);
    }

    position.y += velocity.y * dt;
    sprite.setPosition(position);
    if (map.isCollidingWith(sprite.getGlobalBounds())) {
        position.y -= velocity.y * dt;
        sprite.setPosition(position);
    }
}

void Player::draw(sf::RenderWindow& window) {
    window.draw(sprite);
}

void Player::takeDamage() {
    health--;
}

int   Player::getHealth()       const { return health; }
int   Player::getDiaryPages()   const { return diaryPages; }
int   Player::getSaltLanterns() const { return saltLanterns; }
float Player::getStamina()      const { return stamina; }
