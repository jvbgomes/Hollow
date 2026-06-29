#include "AudioManager.hpp"

// ── Caminhos dos arquivos ─────────────────────────────────────────────────────

std::string AudioManager::musicPath(MusicTrack track) {
    switch (track) {
        case MusicTrack::Menu:     return "assets/audio/music/menu.ogg";
        case MusicTrack::Intro:    return "assets/audio/music/intro.ogg";
        case MusicTrack::Explore:  return "assets/audio/music/explore.ogg";
        case MusicTrack::Dialogue: return "assets/audio/music/dialogue.ogg";
        case MusicTrack::EnemyNearby: return "assets/audio/music/enemy_nearby.ogg";
        case MusicTrack::Chase:    return "assets/audio/music/chase.ogg";
        case MusicTrack::Boss:     return "assets/audio/music/boss.ogg";
        case MusicTrack::Victory:  return "assets/audio/music/victory.ogg";
        case MusicTrack::GameOver: return "assets/audio/music/gameover.ogg";
        default: return "";
    }
}

std::string AudioManager::sfxPath(SfxId id) {
    switch (id) {
        case SfxId::Footstep:      return "assets/audio/sfx/footstep.ogg";
        case SfxId::LanternThrow:  return "assets/audio/sfx/lantern_throw.wav";
        case SfxId::LanternImpact: return "assets/audio/sfx/lantern_impact.wav";
        case SfxId::ItemCollect:   return "assets/audio/sfx/item_collect.wav";
        case SfxId::PageCollect:   return "assets/audio/sfx/page_collect.wav";
        case SfxId::Damage:        return "assets/audio/sfx/damage.wav";
        case SfxId::NpcInteract:   return "assets/audio/sfx/npc_interact.wav";
        case SfxId::BossGrowl:    return "assets/audio/sfx/boss_growl.ogg";
        case SfxId::BookOpen:     return "assets/audio/sfx/book_open.wav";
        case SfxId::BookClose:    return "assets/audio/sfx/book_close.wav";
        case SfxId::CursorMove:   return "assets/audio/sfx/cursor_move.wav";
        case SfxId::Drink:        return "assets/audio/sfx/drink.wav";
        default: return "";
    }
}

// ── Construtor — pré-carrega todos os SFX ────────────────────────────────────

AudioManager::AudioManager() {
    for (int i = (int)SfxId::Footstep; i <= (int)SfxId::Drink; ++i)
        loadSfx(static_cast<SfxId>(i));
}

void AudioManager::loadSfx(SfxId id) {
    int key = (int)id;
    std::string path = sfxPath(id);
    if (path.empty()) return;

    m_buffers[key] = sf::SoundBuffer();
    if (!m_buffers[key].loadFromFile(path)) {
        m_buffers.erase(key);   // arquivo não encontrado — ignora silenciosamente
        return;
    }
    m_sounds[key].setBuffer(m_buffers[key]);
}

// ── Música com crossfade ──────────────────────────────────────────────────────

void AudioManager::playMusic(MusicTrack track, float volume) {
    if (track == m_curTrack && !m_crossfading) return;
    if (track == m_nextTrack && m_crossfading) return;

    m_targetVolume = volume;

    std::string path = musicPath(track);
    if (path.empty()) return;

    // Primeiro track: sem crossfade, toca direto em volume cheio
    if (m_curTrack == MusicTrack::None) {
        if (!m_music[m_cur].openFromFile(path)) return;
        m_music[m_cur].setLoop(true);
        m_music[m_cur].setVolume(volume);
        m_music[m_cur].play();
        m_curTrack = track;
        return;
    }

    int next = 1 - m_cur;
    if (!m_music[next].openFromFile(path)) return;

    m_music[next].setLoop(true);
    m_music[next].setVolume(0.f);
    m_music[next].play();

    m_nextTrack  = track;
    m_crossfading = true;
    m_fadeTimer   = 0.f;
}

void AudioManager::update(float dt) {
    if (!m_crossfading) return;

    m_fadeTimer += dt;
    float t = m_fadeTimer / m_fadeDuration;
    if (t >= 1.f) t = 1.f;

    int next = 1 - m_cur;
    m_music[m_cur] .setVolume(m_targetVolume * (1.f - t));
    m_music[next]  .setVolume(m_targetVolume * t);

    if (t >= 1.f) {
        m_music[m_cur].stop();
        m_cur        = next;
        m_curTrack   = m_nextTrack;
        m_nextTrack  = MusicTrack::None;
        m_crossfading = false;
    }
}

void AudioManager::stopMusic() {
    m_music[0].stop();
    m_music[1].stop();
    m_curTrack   = MusicTrack::None;
    m_crossfading = false;
}

void AudioManager::setMusicVolume(float volume) {
    m_targetVolume = volume;
    if (!m_crossfading)
        m_music[m_cur].setVolume(volume);
}

// ── SFX ───────────────────────────────────────────────────────────────────────

void AudioManager::playSfx(SfxId id, float volume) {
    int key = (int)id;
    auto it = m_sounds.find(key);
    if (it == m_sounds.end()) return;
    it->second.setVolume(volume);
    it->second.play();
}

void AudioManager::stopSfx(SfxId id) {
    int key = (int)id;
    auto it = m_sounds.find(key);
    if (it != m_sounds.end())
        it->second.stop();
}

// ── Passos (controle de intervalo) ───────────────────────────────────────────

void AudioManager::updateFootstep(bool moving, bool sprinting, float dt) {
    if (!moving) {
        m_footstepTimer = 0.f;
        m_wasMoving = false;
        stopSfx(SfxId::Footstep);
        return;
    }

    // Toca imediatamente ao começar a mover
    if (!m_wasMoving) {
        playSfx(SfxId::Footstep, 60.f);
        m_footstepTimer = 0.f;
        m_wasMoving = true;
        return;
    }

    m_footstepTimer += dt;
    float interval = sprinting ? 0.28f : 0.42f;

    if (m_footstepTimer >= interval) {
        playSfx(SfxId::Footstep, 60.f);
        m_footstepTimer = 0.f;
    }
}
