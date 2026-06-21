#pragma once

#include <vector>
#include <string>
#include "Entity.hpp"

class NPC : public Entity {
private:
    std::string name;
    std::vector<std::vector<std::string>> dialogueQueues;
    float interactionRadius;

public:
    NPC(std::string npcName, float x, float y, const sf::Texture& npcTexture, sf::IntRect textureRect);
    ~NPC() override = default;

    void update(float deltaTime, const Map& map, sf::Vector2f playerPosition) override;
    void draw(sf::RenderWindow& window) override;

    void addDialogueOption(int optionIndex, const std::vector<std::string>& sentences);
    std::string getNextSentence(int optionIndex);
    bool hasDialogue(int optionIndex) const;

    bool isPlayerNearby(sf::Vector2f playerPosition) const;
    std::string getName() const;
};