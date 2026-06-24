#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include "DialogueLine.hpp"

class DialogueBox {
public:
    DialogueBox();
    ~DialogueBox() = default;

    void loadFont(const std::string& fontPath);

    // Abre a lista de perguntas (com flag 'seen' para cinzar as já vistas)
    void startOptions(const std::string& npcName, const sf::Texture* tex,
                      sf::IntRect rect,
                      const std::vector<DialogueMenuOption>& opts);

    void moveCursor(int delta);        // -1 = cima, +1 = baixo
    int  confirmOption();              // retorna índice da opção (-1 = Sair)
    void updateOptionSeen(int idx);    // marca opção como vista no menu salvo

    // Exibe sequência de falas alternando investigador/NPC
    void startResponse(const DialogueSequence& lines,
                       const sf::Texture* npcTex,  sf::IntRect npcRect,
                       const sf::Texture* playerTex, sf::IntRect playerRect);

    void advanceDialogue();
    void update(float dt);
    void draw(sf::RenderWindow& window);

    bool isActive()         const;
    bool isShowingOptions() const;
    bool isTextFinished()   const;
    void close();

private:
    enum class Phase { None, Options, Response };
    Phase m_phase = Phase::None;

    sf::Font           m_font;
    sf::RectangleShape m_background;
    sf::RectangleShape m_portraitBorder;
    sf::Sprite         m_portrait;
    sf::Text           m_nameText;
    sf::Text           m_dialogueText;

    // Fase de opções
    std::vector<DialogueMenuOption> m_menuOpts;
    std::vector<sf::Text>           m_optionTexts;
    int                             m_cursor = 0;

    // Fase de resposta (sequência de falas)
    DialogueSequence  m_sequence;
    int               m_seqIdx       = 0;
    sf::String        m_fullLineSF;   // UTF-32 para typewriter correto
    sf::String        m_displayedSF;
    float             m_textSpeed    = 0.04f;
    float             m_textTimer    = 0.f;
    size_t            m_charIdx      = 0;
    bool              m_lineFinished = false;

    const sf::Texture* m_npcTex    = nullptr;
    sf::IntRect        m_npcRect;
    const sf::Texture* m_playerTex = nullptr;
    sf::IntRect        m_playerRect;

    void setupPortrait(bool isPlayer);
    void rebuildOptionTexts();
    void beginCurrentLine();
    void returnToOptions();

    // Dados salvos para retornar às opções após uma resposta
    std::string                      m_savedNpcName;
    const sf::Texture*               m_savedNpcTex  = nullptr;
    sf::IntRect                      m_savedNpcRect;
    std::vector<DialogueMenuOption>  m_savedOpts;
};
