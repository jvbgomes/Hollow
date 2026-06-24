#include "NPC.hpp"
#include <cmath>

static const DialogueSequence EMPTY_SEQ;

NPC::NPC(std::string npcName, float x, float y, const sf::Texture& npcTexture, sf::IntRect textureRect)
    : Entity(x, y), name(npcName), interactionRadius(32.f),
      m_texPtr(&npcTexture), m_portraitRect(textureRect)
{
    sprite.setTexture(npcTexture);
    sprite.setTextureRect(textureRect);
    sprite.setPosition(position);
}

void NPC::update(float, const Map&, sf::Vector2f) {
    sprite.setPosition(position);
}

void NPC::draw(sf::RenderTarget& target) {
    target.draw(sprite);
}

void NPC::addOption(const std::string& label, const DialogueSequence& lines) {
    m_options.push_back({label, lines, false});
}

const DialogueSequence& NPC::getOptionLines(int idx) const {
    if (idx < 0 || idx >= (int)m_options.size()) return EMPTY_SEQ;
    return m_options[idx].lines;
}

void NPC::markSeen(int idx) {
    if (idx >= 0 && idx < (int)m_options.size())
        m_options[idx].seen = true;
}

bool NPC::isOptionSeen(int idx) const {
    if (idx < 0 || idx >= (int)m_options.size()) return false;
    return m_options[idx].seen;
}

std::vector<DialogueMenuOption> NPC::getAvailableOptions() const {
    std::vector<DialogueMenuOption> result;
    for (int i = 0; i < (int)m_options.size(); ++i)
        result.push_back({i, m_options[i].label, m_options[i].seen});
    return result;
}

bool NPC::isPlayerNearby(sf::Vector2f pos) const {
    float dx = position.x - pos.x;
    float dy = position.y - pos.y;
    return std::sqrt(dx*dx + dy*dy) <= interactionRadius;
}

std::string        NPC::getName()         const { return name; }
const sf::Texture* NPC::getTexture()      const { return m_texPtr; }
sf::IntRect        NPC::getPortraitRect() const { return m_portraitRect; }
