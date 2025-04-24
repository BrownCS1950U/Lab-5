#include "audio.h"
#include <glm/glm.hpp>
#include <algorithm>
#include <stdexcept>

using namespace gl;

/* singleton ------------------------------------------------------- */
AudioEngine& AudioEngine::instance() {
    static AudioEngine e;
    return e;
}

/* ctor/dtor ------------------------------------------------------- */
AudioEngine::AudioEngine() {}
AudioEngine::~AudioEngine() { shutdown(); }

/* device life-cycle ---------------------------------------------- */
void AudioEngine::init() {
    if (m_initialized) return;
    InitAudioDevice();
    if (!IsAudioDeviceReady())
        throw std::runtime_error("Audio device init failed");
    m_initialized = true;
}
void AudioEngine::shutdown() {
    if (!m_initialized) return;

    for (auto& a : m_active) UnloadSoundAlias(a.alias);
    for (auto& [_, s] : m_sounds) UnloadSound(s.sound);
    for (auto& [_, m] : m_musics) UnloadMusicStream(m.music);

    CloseAudioDevice();
    m_active.clear(); m_sounds.clear(); m_musics.clear();
    m_initialized = false;
}
bool AudioEngine::ready() const { return m_initialized; }

/* loading --------------------------------------------------------- */
void AudioEngine::loadSound(const std::string& p,const std::string& n) {
    unloadSound(n);
    Sound s = LoadSound(p.c_str());
    if (!IsSoundReady(s))
        throw std::runtime_error("Failed to load sound: "+p);
    m_sounds[n] = {s};
}
void AudioEngine::loadMusic(const std::string& p,const std::string& n,bool loop) {
    unloadMusic(n);
    Music m = LoadMusicStream(p.c_str());
    if (!IsMusicReady(m))
        throw std::runtime_error("Failed to load music: "+p);
    m.looping = loop;
    m_musics[n] = {m};
}
void AudioEngine::unloadSound(const std::string& n) {
    auto it = m_sounds.find(n);
    if (it!=m_sounds.end()) { UnloadSound(it->second.sound); m_sounds.erase(it); }
}
void AudioEngine::unloadMusic(const std::string& n) {
    auto it = m_musics.find(n);
    if (it!=m_musics.end()){ UnloadMusicStream(it->second.music); m_musics.erase(it); }
}

/* 2-D playback ---------------------------------------------------- */
void AudioEngine::playSound(const std::string& n,float v,float pitch,float pan) {
    auto it = m_sounds.find(n); if (it==m_sounds.end()) return;
    Sound alias = LoadSoundAlias(it->second.sound);
    SetSoundVolume(alias,v); SetSoundPitch(alias,pitch); SetSoundPan(alias,pan);
    PlaySound(alias);
    m_active.push_back({alias,{},false});
}

/* 3-D playback ---------------------------------------------------- */
void AudioEngine::playSound3D(const std::string& n,const glm::vec3& pos,float pitch) {
    auto it = m_sounds.find(n); if (it==m_sounds.end()) return;
    Sound alias = LoadSoundAlias(it->second.sound);
    SetSoundPitch(alias,pitch);
    PlaySound(alias);
    m_active.push_back({alias,pos,true});
}

/* music ----------------------------------------------------------- */
void AudioEngine::playMusic(const std::string& n)   { auto i=m_musics.find(n); if(i!=m_musics.end()) PlayMusicStream(i->second.music); }
void AudioEngine::pauseMusic(const std::string& n)  { auto i=m_musics.find(n); if(i!=m_musics.end()) PauseMusicStream(i->second.music);}
void AudioEngine::resumeMusic(const std::string& n) { auto i=m_musics.find(n); if(i!=m_musics.end()) ResumeMusicStream(i->second.music);}
void AudioEngine::stopMusic(const std::string& n)   { auto i=m_musics.find(n); if(i!=m_musics.end()) StopMusicStream(i->second.music); }
void AudioEngine::seekMusic(const std::string& n, float t){auto i=m_musics.find(n);if(i!=m_musics.end()) SeekMusicStream(i->second.music,t);}

/* per-asset setters (unchanged) ---------------------------------- */
#define SETTER(fn,map) auto it=map.find(name); if(it!=map.end()) fn
void AudioEngine::setSoundVolume(const std::string& name,float v){SETTER(SetSoundVolume(it->second.sound,v),m_sounds);}
void AudioEngine::setSoundPitch (const std::string& name,float p){SETTER(SetSoundPitch (it->second.sound,p),m_sounds);}
void AudioEngine::setSoundPan   (const std::string& name,float x){SETTER(SetSoundPan   (it->second.sound,x),m_sounds);}
void AudioEngine::setMusicVolume(const std::string& name,float v){SETTER(SetMusicVolume(it->second.music,v),m_musics);}
void AudioEngine::setMusicPitch (const std::string& name,float p){SETTER(SetMusicPitch (it->second.music,p),m_musics);}
void AudioEngine::setMusicPan   (const std::string& name,float x){SETTER(SetMusicPan   (it->second.music,x),m_musics);}
#undef SETTER

/* master ---------------------------------------------------------- */
void  AudioEngine::setMasterVolume(float v){ SetMasterVolume(v);}
float AudioEngine::masterVolume()   { return GetMasterVolume(); }

/* listener -------------------------------------------------------- */
void AudioEngine::setListener(const glm::vec3& p) { m_listener=p; }
void AudioEngine::setMaxDistance(float m)         { m_maxDist = std::max(0.1f,m); }

/* update ---------------------------------------------------------- */
void AudioEngine::update() {
    /* retire or refresh aliases */
    for (size_t i=0;i<m_active.size();) {
        auto& a = m_active[i];
        if(!IsSoundPlaying(a.alias))
        {   UnloadSoundAlias(a.alias); m_active[i]=m_active.back(); m_active.pop_back(); continue; }

        if(a.spatial) updateAliasSpatial(a);
        ++i;
    }

    /* feed music streams */
    for(auto& [_,m]:m_musics) {
        UpdateMusicStream(m.music);
        if(!musicIsPlaying(m)&&m.music.looping){SeekMusicStream(m.music,0);PlayMusicStream(m.music);}
    }
}

/* spatial helper -------------------------------------------------- */
void AudioEngine::updateAliasSpatial(ActiveAlias& a) {
    glm::vec3 toSrc = a.pos - m_listener;
    float dist      = glm::length(toSrc);
    float vol       = std::clamp(1.f - dist/m_maxDist, 0.f, 1.f);

    float pan = 0.5f;                    // default centre
    if (dist > 0.0001f) {
        float x = toSrc.x / dist;        // -1 .. +1 (rough “left-right”)
        pan = std::clamp(0.5f + 0.5f*x, 0.f, 1.f);
    }

    SetSoundVolume(a.alias, vol);
    SetSoundPan   (a.alias, pan);
}

/* misc helpers ---------------------------------------------------- */
bool AudioEngine::musicIsPlaying(const MusicEntry& e)   { return IsMusicStreamPlaying(e.music); }

/* raw handles ----------------------------------------------------- */
Sound AudioEngine::rawSound(const std::string& n) const { return m_sounds.at(n).sound; }
Music&AudioEngine::rawMusic(const std::string& n)       { return m_musics.at(n).music; }
