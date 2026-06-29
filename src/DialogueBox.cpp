#include "DialogueBox.hpp"
#include <algorithm>

static sf::String u8(const std::string& s) {
    return sf::String::fromUtf8(s.begin(), s.end());
}

static const float BOX_X   = 20.f;
static const float BOX_Y   = 425.f;
static const float BOX_W   = 760.f;
static const float BOX_H   = 160.f;
static const float PORT_X  = 35.f;
static const float PORT_Y  = 440.f;
static const float PORT_SZ = 125.f;
static const float TEXT_X  = 175.f;

DialogueBox::DialogueBox() {
    m_background.setSize({BOX_W, BOX_H});
    m_background.setFillColor(sf::Color(5, 5, 15, 230));
    m_background.setOutlineColor(sf::Color(120, 120, 160));
    m_background.setOutlineThickness(2.f);
    m_background.setPosition(BOX_X, BOX_Y);

    m_portraitBorder.setSize({PORT_SZ, PORT_SZ});
    m_portraitBorder.setFillColor(sf::Color(15, 15, 25));
    m_portraitBorder.setOutlineColor(sf::Color(150, 150, 200));
    m_portraitBorder.setOutlineThickness(2.f);
    m_portraitBorder.setPosition(PORT_X, PORT_Y);

    m_nameText.setCharacterSize(18);
    m_nameText.setStyle(sf::Text::Bold);
    m_nameText.setPosition(TEXT_X, BOX_Y + 10.f);

    m_dialogueText.setCharacterSize(16);
    m_dialogueText.setFillColor(sf::Color::White);
    m_dialogueText.setPosition(TEXT_X, BOX_Y + 38.f);
}

void DialogueBox::loadFont(const std::string& path) {
    if (m_font.loadFromFile(path)) {
        m_nameText.setFont(m_font);
        m_dialogueText.setFont(m_font);
    }
}

void DialogueBox::setupPortrait(bool isPlayer) {
    const sf::Texture* tex  = isPlayer ? m_playerTex : m_npcTex;
    sf::IntRect        rect = isPlayer ? m_playerRect : m_npcRect;
    if (!tex || rect.width == 0 || rect.height == 0) return;
    m_portrait.setTexture(*tex);
    m_portrait.setTextureRect(rect);
    float scale = std::min(PORT_SZ / rect.width, PORT_SZ / rect.height);
    m_portrait.setScale(scale, scale);
    float drawW = rect.width  * scale;
    float drawH = rect.height * scale;
    m_portrait.setPosition(PORT_X + (PORT_SZ - drawW) / 2.f,
                           PORT_Y + (PORT_SZ - drawH) / 2.f);
}

// ---------- Options phase ----------

void DialogueBox::startOptions(const std::string& npcName, const sf::Texture* tex,
                                sf::IntRect rect,
                                const std::vector<DialogueMenuOption>& opts) {
    // Salva dados para retornar depois de uma resposta
    m_savedNpcName = npcName;
    m_savedNpcTex  = tex;
    m_savedNpcRect = rect;
    m_savedOpts    = opts;

    m_phase    = Phase::Options;
    m_cursor   = 0;
    m_menuOpts = opts;

    // Adiciona opção "Sair" com índice sentinela -1
    DialogueMenuOption exitOpt;
    exitOpt.index = -1;
    exitOpt.label = "Sair";
    exitOpt.seen  = false;
    m_menuOpts.push_back(exitOpt);

    m_npcTex   = tex;
    m_npcRect  = rect;
    m_nameText.setString(u8(npcName));
    m_nameText.setFillColor(sf::Color::Cyan);
    setupPortrait(false);
    rebuildOptionTexts();
}

void DialogueBox::returnToOptions() {
    if (m_savedOpts.empty()) {
        m_phase = Phase::None;
        return;
    }

    m_phase    = Phase::Options;
    m_cursor   = 0;
    m_menuOpts = m_savedOpts;

    DialogueMenuOption exitOpt;
    exitOpt.index = -1;
    exitOpt.label = "Sair";
    exitOpt.seen  = false;
    m_menuOpts.push_back(exitOpt);

    m_npcTex  = m_savedNpcTex;
    m_npcRect = m_savedNpcRect;
    m_nameText.setString(u8(m_savedNpcName));
    m_nameText.setFillColor(sf::Color::Cyan);
    setupPortrait(false);
    rebuildOptionTexts();
}

void DialogueBox::updateOptionSeen(int optionIdx) {
    for (auto& o : m_savedOpts)
        if (o.index == optionIdx) { o.seen = true; break; }
}

void DialogueBox::rebuildOptionTexts() {
    m_optionTexts.clear();
    float y = BOX_Y + 44.f;
    for (int i = 0; i < (int)m_menuOpts.size(); ++i) {
        sf::Text t;
        t.setFont(m_font);
        t.setCharacterSize(16);
        t.setPosition(TEXT_X + 8.f, y);

        bool selected = (i == m_cursor);
        bool seen     = m_menuOpts[i].seen;

        std::string prefix = selected ? "> " : "  ";
        t.setString(u8(prefix + m_menuOpts[i].label));

        if (selected)
            t.setFillColor(seen ? sf::Color(200, 200, 80) : sf::Color::Yellow);
        else
            t.setFillColor(seen ? sf::Color(90, 90, 90) : sf::Color(200, 200, 200));

        m_optionTexts.push_back(t);
        y += 26.f;
    }
}

void DialogueBox::moveCursor(int delta) {
    if (m_menuOpts.empty()) return;
    m_cursor = (m_cursor + delta + (int)m_menuOpts.size()) % (int)m_menuOpts.size();
    rebuildOptionTexts();
}

int DialogueBox::confirmOption() {
    int chosen = (m_cursor < (int)m_menuOpts.size()) ? m_menuOpts[m_cursor].index : -1;
    if (chosen == -1) m_phase = Phase::None;   // "Sair" fecha
    // Para outros índices, o caller chama startResponse e muda a fase
    return chosen;
}

// ---------- Response phase ----------

void DialogueBox::startResponse(const DialogueSequence& lines,
                                 const sf::Texture* npcTex,  sf::IntRect npcRect,
                                 const sf::Texture* playerTex, sf::IntRect playerRect) {
    if (lines.empty()) { m_phase = Phase::None; return; }
    m_phase      = Phase::Response;
    m_sequence   = lines;
    m_seqIdx     = 0;
    m_npcTex     = npcTex;
    m_npcRect    = npcRect;
    m_playerTex  = playerTex;
    m_playerRect = playerRect;
    beginCurrentLine();
}

void DialogueBox::beginCurrentLine() {
    const DialogueLine& line = m_sequence[m_seqIdx];
    m_fullLineSF   = u8(line.text);
    m_displayedSF  = sf::String();
    m_charIdx      = 0;
    m_textTimer    = 0.f;
    m_lineFinished = false;
    m_dialogueText.setString(sf::String());

    m_nameText.setString(u8(line.speaker));
    m_nameText.setFillColor(line.isPlayer ? sf::Color(255, 220, 100) : sf::Color::Cyan);
    setupPortrait(line.isPlayer);
}

void DialogueBox::advanceDialogue() {
    if (m_phase != Phase::Response) return;
    if (!m_lineFinished) {
        m_lineFinished = true;
        m_dialogueText.setString(m_fullLineSF);
    } else {
        m_seqIdx++;
        if (m_seqIdx < (int)m_sequence.size()) {
            beginCurrentLine();
        } else {
            returnToOptions();   // volta ao menu de perguntas em vez de fechar
        }
    }
}

void DialogueBox::update(float dt) {
    if (m_phase != Phase::Response || m_lineFinished) return;
    m_textTimer += dt;
    if (m_textTimer >= m_textSpeed) {
        m_textTimer = 0.f;
        if (m_charIdx < m_fullLineSF.getSize()) {
            m_displayedSF += m_fullLineSF[m_charIdx++];
            m_dialogueText.setString(m_displayedSF);
        } else {
            m_lineFinished = true;
        }
    }
}

void DialogueBox::draw(sf::RenderWindow& window) {
    if (m_phase == Phase::None) return;
    window.draw(m_background);
    window.draw(m_portraitBorder);
    window.draw(m_portrait);
    window.draw(m_nameText);
    if (m_phase == Phase::Options) {
        for (auto& t : m_optionTexts) window.draw(t);
    } else {
        window.draw(m_dialogueText);
    }
}

bool DialogueBox::isActive()         const { return m_phase != Phase::None; }
bool DialogueBox::isShowingOptions() const { return m_phase == Phase::Options; }
bool DialogueBox::isTextFinished()   const { return m_lineFinished; }
void DialogueBox::close()                  { m_phase = Phase::None; }
