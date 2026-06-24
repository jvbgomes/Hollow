#pragma once
#include <SFML/Audio.hpp>
#include <string>
#include <unordered_map>

enum class MusicTrack {
    None,
    Menu,
    Intro,
    Explore,
    Dialogue,
    EnemyNearby,
    Chase,
    Boss,
    Victory,
    GameOver
};

enum class SfxId {
    Footstep,
    LanternThrow,
    LanternImpact,
    ItemCollect,
    PageCollect,
    Damage,
    NpcInteract,
    BossGrowl,
    BookOpen,
    BookClose,
    CursorMove
};

class AudioManager {
public:
    AudioManager();

    // Deve ser chamado todo frame (gerencia crossfade)
    void update(float dt);

    // Solicita troca de trilha com crossfade suave
    void playMusic(MusicTrack track, float volume = 40.f);
    void stopMusic();
    void setMusicVolume(float volume);

    // Toca / para um efeito sonoro
    void playSfx(SfxId id, float volume = 80.f);
    void stopSfx(SfxId id);

    // Deve ser chamado no update para sons de passo (controle de intervalo)
    void updateFootstep(bool moving, bool sprinting, float dt);

private:
    // Dois streams para crossfade
    sf::Music  m_music[2];
    int        m_cur        = 0;       // índice do stream ativo
    MusicTrack m_curTrack   = MusicTrack::None;
    MusicTrack m_nextTrack  = MusicTrack::None;

    bool  m_crossfading  = false;
    float m_fadeTimer    = 0.f;
    float m_fadeDuration = 1.5f;      // segundos de crossfade
    float m_targetVolume = 40.f;

    std::unordered_map<int, sf::SoundBuffer> m_buffers;
    std::unordered_map<int, sf::Sound>       m_sounds;

    float m_footstepTimer = 0.f;
    bool  m_wasMoving     = false;

    static std::string musicPath(MusicTrack track);
    static std::string sfxPath(SfxId id);
    void loadSfx(SfxId id);
};
