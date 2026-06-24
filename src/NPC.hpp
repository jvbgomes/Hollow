#pragma once

#include <vector>
#include <string>
#include "Entity.hpp"
#include "DialogueLine.hpp"

class NPC : public Entity {
public:
    NPC(std::string npcName, float x, float y, const sf::Texture& npcTexture, sf::IntRect textureRect);
    ~NPC() override = default;

    void update(float dt, const Map& map, sf::Vector2f playerPosition) override;
    void draw(sf::RenderTarget& target) override;

    void addOption(const std::string& label, const DialogueSequence& lines);

    const DialogueSequence& getOptionLines(int idx) const;
    void markSeen(int idx);
    bool isOptionSeen(int idx) const;

    std::vector<DialogueMenuOption> getAvailableOptions() const;

    bool               isPlayerNearby(sf::Vector2f playerPosition) const;
    std::string        getName()         const;
    const sf::Texture* getTexture()      const;
    sf::IntRect        getPortraitRect() const;

private:
    struct Option {
        std::string      label;
        DialogueSequence lines;
        bool             seen = false;
    };

    std::string         name;
    std::vector<Option> m_options;
    float               interactionRadius;
    const sf::Texture*  m_texPtr      = nullptr;
    sf::IntRect         m_portraitRect;
};
