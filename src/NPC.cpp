#include "NPC.hpp"
#include <cmath>

NPC::NPC(std::string npcName, float x, float y, const sf::Texture& npcTexture, sf::IntRect textureRect)
    : Entity(x, y), name(npcName), interactionRadius(64.0f) {
    sprite.setTexture(npcTexture);
    sprite.setTextureRect(textureRect);
    sprite.setPosition(position);
    dialogueQueues.resize(3);
}

void NPC::update(float deltaTime, const Map& map, sf::Vector2f playerPosition) {
    sprite.setPosition(position);
}

void NPC::draw(sf::RenderWindow& window) {
    window.draw(sprite);
}

void NPC::addDialogueOption(int optionIndex, const std::vector<std::string>& sentences) {
    if (optionIndex >= 0 && optionIndex < static_cast<int>(dialogueQueues.size())) {
        dialogueQueues[optionIndex] = sentences;
    }
}

std::string NPC::getNextSentence(int optionIndex) {
    if (!hasDialogue(optionIndex)) {
        return "";
    }
    
    std::string nextSentence = dialogueQueues[optionIndex].front();
    dialogueQueues[optionIndex].erase(dialogueQueues[optionIndex].begin());
    return nextSentence;
}

bool NPC::hasDialogue(int optionIndex) const {
    if (optionIndex >= 0 && optionIndex < static_cast<int>(dialogueQueues.size())) {
        return !dialogueQueues[optionIndex].empty();
    }
    return false;
}

bool NPC::isPlayerNearby(sf::Vector2f playerPosition) const {
    float dx = position.x - playerPosition.x;
    float dy = position.y - playerPosition.y;
    float distance = std::sqrt(dx * dx + dy * dy);
    return distance <= interactionRadius;
}

std::string NPC::getName() const {
    return name;
}